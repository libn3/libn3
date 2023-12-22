#pragma once

#include <functional>
#include <tuple>

#include "buffer.h"
#include "epoll.h"
#include "error.h"

/**
 * Runtime is the meta glue layer between top level application usage and internals such as
 * epoll, raw TCP sockets, syscalls, and other details
 */

namespace n3::runtime {

//TODO: This is a one-time-only callback, do we want a second potential type for multi-call?
//TODO: This has issues with void/empty call args like the result of std::bind to collapse them all
template<typename... cArgs>
class callback {
    std::move_only_function<void(cArgs...)> mf;

public:
    //TODO: Verify requirements on std::move_only_function to prevent UB (like ensure Func type is constructible)
    template<typename F, typename... fArgs>
    callback(F&& func, fArgs&&...func_args) :
            mf{std::bind_front(std::forward<F>(func), std::forward<fArgs...>(func_args...))} {
    }

    callback(const callback&) noexcept = delete;
    callback(callback&&) noexcept = default;

    callback& operator=(const callback&) noexcept = delete;
    callback& operator=(callback&&) noexcept = default;

    /*
     * The && at the end signifies that this is only callable by rvalue references of *this
     * It's a way to specify overload resolution based on reference type
     * And with it added explicitly, that means the lvalue reference call doesn't exist, so we get
     * a compile time error if the user doesn't std::move() the object when calling the callback
     */
    void operator()(cArgs&&...call_args) && noexcept(
            noexcept(this->mf(std::forward<cArgs...>(call_args...)))) {
        std::move(this->mf)(std::forward<cArgs...>(call_args...));
        //TODO: Would like to do something like empty out the function after usage by setting nullptr
        //Problem is, calling operator() on an empty std::move_only_function is UB, so I'd have to throw something myself
    }
};

/*
 * TODO: Need to think about things in terms of submission/completion queues
 * That's technically a "Proactor" software design pattern, but I'm not caring about that at all
 * It's useful to think of because it encapsulates everything I need to work with
 * Namely, submission queue for epoll stuff, completion queue for responding to events and callbacks
 * Also scales well outside of epoll due to it natively matching io_uring and IOCP designs
 * So the question becomes:
 * How to design the system around submission/completion queues sanely?
 * How does this play into the single vs multithreaded runtime?
 * Using queues in a single threaded runtime feels like pointless copying/overhead
 * Do we share completion queues between worker threads, or have 1 queue per?
 * Do we care about cross-thread IO vs callback mechanisms?
 * For example thread 1 does recv(), thread 2 calls callback with data from completion queue
 * How do I want to consolidate submission queues per-object to allow transparent scatter-gather IO?
 *
 * If I build this out of ring buffers, what do I do when it's full and trying to overwrite?
 * What is the memory allocation landscape going to look like here?
 * Is this "easy" with EPOLLONESHOT, or does that add too much overhead in syscalls?
 * Having a "strand" type mechanism sounds useful, but how would that play with these queues/events?
 */

//template<typename T>
//class event {
//    T handle;
//
//public:
//    event() = default;
//};

//TODO: Add configuration options through some init/builder/option struct pattern
//TODO: Add multithreaded runtime
//TODO: Naming?

class runtime_st {
    n3::linux::epoll::epoll_ctx epoll;
    bool active;

    n3::PageBuffer read_buffer;

public:
    runtime_st() noexcept;

    void run();
};

} // namespace n3::runtime

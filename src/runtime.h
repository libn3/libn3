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
template<typename... cArgs>
class callback {
    std::move_only_function<void(cArgs...)> mf;

public:
    template<typename F, typename... fArgs>
    callback(F&& func, fArgs&&...func_args) : mf{std::bind_front(func, func_args...)} {
    }

    callback(const callback&) = delete;
    callback(callback&&) noexcept = default;

    callback& operator=(const callback&) = delete;
    callback& operator=(callback&&) = default;

    /*
     * The && at the end signifies that this is only callable by rvalue references of *this
     * It's a way to specify overload resolution based on reference type
     * And with it added explicitly, that means the lvalue reference call doesn't exist, so we get
     * a compile time error if the user doesn't std::move() the object when calling the callback
     */
    void operator()(cArgs&&...call_args) && noexcept(this->mf()) {
        std::move(this->mf)(call_args...);
    }
};

//TODO: Add configuration options through some init/builder/option struct pattern
//TODO: Add multithreaded runtime
//TODO: Naming?

class runtime_st {
    n3::epoll::epoll_ctx epoll;
    bool active;

    n3::PageBuffer read_buffer;

public:
    runtime_st() noexcept;

    void run();
};

} // namespace n3::runtime

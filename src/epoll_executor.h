#pragma once

#include <coroutine>
#include <exception>
#include <expected>
#include <unordered_map>

#include "buffer.h"
#include "epoll.h"
#include "error.h"
#include "handle.h"

namespace n3::linux::epoll {

//TODO: Naming
//TODO: Anything else needed to be stored here?
//TODO: Encapsulation semantics or RAII useful here?
struct epoll_handle_state {
    //Not an OwnedHandle because this is a weak reference that should not affect lifetimes
    Handle fd;
    BufferQueue tx_queue;
    BufferQueue rx_queue;
    //Whether the handle is known to be readable/writable already (did we hit EAGAIN after epoll?)
    bool tx_active;
    bool rx_active;
};

/*
 * TODO: Epoll executor needs to be fleshed out in 2 main ways:
 *  - Epoll event generator range algorithm thing
 *  - Epoll coroutine work for actual usage by library/application code
 *      - For example: co_await Handle.is_writable();
 *  - Timer timestamp max heap vector for the application timer list used for epoll_wait timeouts
 *      - Likely will require timer coroutines of some kind as a future extension, so don't make it too basic
 *
 * Timers were the key issue in figuring out next steps here
 * std::generator uses the function it's returned from to work, which means you can only pass in
 * one set of arguments when initializing the generator, so trying to swap out the expiry time to
 * the raw underlying syscall would be really annoying or impossible.
 *
 * Instead, with a timer list, I can just check that for the timeout value, and turn the whole
 * thing into a basic loop instead of worrying about anything else there
 * The reason for an application max heap timer list construction over something like timerfd
 * was to avoid unnecessary fds being created and scaling with timer usage
 * You don't want to run into issues because you added just 1 too many timers and now you've hit
 * rlimit max errors or something
 */
class epoll_executor {
    epoll_ctx epoll;
    bool active;

    std::unordered_map<Handle, epoll_handle_state> handle_map;

    /*
     * TODO: I can't implement this yet because it's too bleeding edge...
     * std::generator support is only added in GCC 14, and that hasn't been released yet.
     * So in a few weeks/months whenever that does happen, I can restart this work, but until then
     * I don't think I want to build a custom generator coroutine object for something about to
     * be available from the standard.
     */
    /*
    [[nodiscard]] auto test() -> std::generator<
            std::expected<const std::span<const ::epoll_event>, error::ErrorCode>>;
    */

public:
    epoll_executor();

    [[nodiscard]] auto add(Handle fd) noexcept -> const std::expected<void, error::ErrorCode>;
    [[nodiscard]] auto remove(Handle fd) noexcept -> const std::expected<void, error::ErrorCode>;

    /*
     * TODO: Need a few more functions
     *  - Run (main loop invocation, may want a run_once split off)
     *  - Registering read/write events and callbacks (Done in an init function, or per-call?)
     *  - Memory buffer to handle data reads that we can return to the user
     */
    void run();
    void run_once();
    void run_until();
};

//TODO: Generalize beyond epoll executor type whenever those exist
//TODO: Naming?
//TODO: Enforce memory lifetimes with refcounting instead of implicitly?
class ExecutorOwnedHandle {
    const OwnedHandle fd;
    epoll_executor& exec;

public:
    explicit ExecutorOwnedHandle(epoll_executor& executor, const Handle fd_arg) :
            fd{fd_arg},
            exec{executor} {
        //.value() on std::expected will throw if the call fails
        this->exec.add(this->fd).value();
    }
    ~ExecutorOwnedHandle() {
        //Don't throw on failure, treat it like closing a socket
        [[maybe_unused]] const auto _ = this->exec.remove(this->fd);
    }

    constexpr ExecutorOwnedHandle(const ExecutorOwnedHandle&) noexcept = delete;
    constexpr ExecutorOwnedHandle(ExecutorOwnedHandle&&) noexcept = default;

    constexpr ExecutorOwnedHandle& operator=(const ExecutorOwnedHandle&) noexcept = delete;
    constexpr ExecutorOwnedHandle& operator=(ExecutorOwnedHandle&&) noexcept = default;

    //Conversion operator to treat this as a plain handle type
    [[nodiscard]] constexpr operator Handle() const noexcept {
        return this->fd;
    }
};

class EpollEventGenerator {
    const epoll_handle efd;

public:
    EpollEventGenerator() noexcept = default;
};

template<typename T>
class EpollTask {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T value;
        std::exception_ptr eptr;

        EpollTask get_return_object() {
            return EpollTask(handle_type::from_promise(*this));
        }
        std::suspend_always initial_suspend() {
            return {};
        }
        std::suspend_always final_suspend() noexcept {
            return {};
        }
        void unhandled_exception() {
            eptr = std::current_exception();
        }
        template<std::convertible_to<T> From>
        std::suspend_always yield_value(From&& from) {
            value = std::forward<From>(from);
            return {};
        }
        void return_void() {
        }
    };

public:
    EpollTask() = default;
};

}; // namespace n3::linux::epoll

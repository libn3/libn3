#pragma once

#include <cstdint>
#include <exception>
#include <expected>
#include <unordered_map>

#include "buffer.h"
#include "epoll.h"
#include "error.h"
#include "handle.h"
#include "ownership.h"

namespace n3::linux::epoll {

//Bit field representing epoll event types as flags
struct events {
    unsigned int in : 1;
    unsigned int out : 1;
    unsigned int rdhup : 1;
    unsigned int pri : 1;
    unsigned int err : 1;
    unsigned int hup : 1;

    constexpr events() noexcept = default;
    constexpr events(const uint32_t epoll_events) noexcept :
            in{epoll_events & EPOLLIN},
            out{epoll_events & EPOLLOUT},
            rdhup{epoll_events & EPOLLRDHUP},
            pri{epoll_events & EPOLLPRI},
            err{epoll_events & EPOLLERR},
            hup{epoll_events & EPOLLHUP} {
    }

    //Sanity check if any events are raised that ARE NOT read or write
    constexpr bool has_error() const noexcept {
        if (this->rdhup) {
            return true;
        }
        if (this->pri) {
            return true;
        }
        if (this->err) {
            return true;
        }
        if (this->hup) {
            return true;
        }
        return false;
    }
};

//TODO: Naming
//TODO: Anything else needed to be stored here?
//TODO: Encapsulation semantics or RAII useful here?
struct epoll_handle_state {
    //Not an OwnedHandle because this is a weak reference that should not affect lifetimes
    Handle fd;
    /*
     * TODO: Buffer queues don't make as much sense here after further thought
     * The main "work" horse here is going to be the read/write coroutines that need to be handled
     * So having a work queue as a FIFO based on user requests is a great plan
     * The issue is that we're doing coroutines, not callbacks, so this doesn't work
     * Additionally, since a read coroutine and a write coroutine are both coroutines that can
     * be type erased between each other, we can flatten them into a single work queue stream
     * of coroutine handles that need to be resumed
     *
     * Can simply model them as "wants read" vs "wants write", and then funnel them that way
     * Any error events call the main work queue head event with the error instead of the normal
     * resumption
     */
    BufferQueue tx_queue;
    BufferQueue rx_queue;
    /*
     * Current known event values for the handle
     * Is updated by returned epoll events and read/write calls hitting EAGAIN
     */
    struct events event_cache;
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

class EpollTask {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    OwnedCoroutine<promise_type> coro_handle;

    struct promise_type {
        struct events epoll_events;
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
        void return_void() {
        }
    };

public:
    EpollTask() noexcept = default;
    EpollTask(handle_type handle) noexcept : coro_handle{handle} {
    }
};

struct EpollAwaitable {
    bool await_ready() {
        return false;
    }
    void await_suspend(std::coroutine_handle<> h) {
        h.resume();
    }
    void await_resume() {
    }
};

}; // namespace n3::linux::epoll

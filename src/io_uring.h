#pragma once

#include <exception>
#include <expected>
#include <liburing.h>

#include "buffer.h"
#include "error.h"
#include "handle.h"
#include "ownership.h"

namespace n3::linux::io_uring {

class io_uring_handle {
    MoveOnly<::io_uring> ring;

public:
    io_uring_handle();
    ~io_uring_handle();

    constexpr const ::io_uring *get() const noexcept {
        return &*ring;
    }
    constexpr ::io_uring *get() noexcept {
        return &*ring;
    }
};

[[nodiscard]] std::optional<std::reference_wrapper<::io_uring_sqe>> get_sqe(
        io_uring_handle& handle) noexcept;

class Executor {
    io_uring_handle uring;
    bool active;

    //std::unordered_map<Handle, epoll_handle_state> handle_map;

    std::vector<Handle> handles;

public:
    Executor();

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

//TODO: Naming?
//TODO: Enforce memory lifetimes with refcounting instead of implicitly?
class ExecutorOwnedHandle {
    const OwnedHandle fd;
    Executor& exec;

public:
    explicit ExecutorOwnedHandle(Executor& executor, const Handle fd_arg) :
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

class Task {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    OwnedCoroutine<promise_type> coro_handle;

    struct promise_type {
        std::exception_ptr eptr;

        Task get_return_object() {
            return Task(handle_type::from_promise(*this));
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
    Task() noexcept = default;
    Task(handle_type handle) noexcept : coro_handle{handle} {
    }

    void execute() {
        while (!coro_handle.done()) {
            coro_handle.resume();
            if (coro_handle.promise().eptr) {
                std::rethrow_exception(coro_handle.promise().eptr);
            }
        }
    }
};

struct UringAwaitable {
    bool await_ready() {
        return false;
    }
    void await_suspend(std::coroutine_handle<> h) {
        h.resume();
    }
    void await_resume() {
    }
};

}; // namespace n3::linux::io_uring

#pragma once

#include <expected>

#include "buffer.h"
#include "epoll.h"
#include "error.h"
#include "handle.h"

namespace n3::linux::epoll {
class epoll_executor {
    epoll_ctx epoll;
    bool active;

    n3::PageBuffer read_buffer;

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
};

//TODO: Generalize beyond epoll executor type whenever those exist
//TODO: Naming?
//TODO: Enforce memory lifetimes with refcounting instead of implicitly?
class ExecutorOwnedHandle {
    const OwnedHandle fd;
    epoll_executor& exec;

public:
    explicit ExecutorOwnedHandle(epoll_executor& executor, const Handle fd_arg) noexcept :
            fd{fd_arg},
            exec{executor} {
        //.value() on std::expected will throw if the call fails
        this->exec.add(this->fd).value();
    }
    ~ExecutorOwnedHandle() noexcept {
        //.value() on std::expected will throw if the call fails
        this->exec.remove(this->fd).value();
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

}; // namespace n3::linux::epoll

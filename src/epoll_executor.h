#pragma once

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

class epoll_executor {
    epoll_ctx epoll;
    bool active;

    std::unordered_map<Handle, epoll_handle_state> handle_map;

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

}; // namespace n3::linux::epoll

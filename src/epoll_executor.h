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

    /*
     * TODO: Need a few more functions
     *  - Run (main loop invocation, may want a run_once split off)
     *  - Registering read/write events and callbacks (Done in an init function, or per-call?)
     *  - Memory buffer to handle data reads that we can return to the user
     */
    void run();
    void run_once();
};
}; // namespace n3::linux::epoll

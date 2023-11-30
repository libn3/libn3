#pragma once

#include <expected>

#include "buffer.h"
#include "epoll.h"
#include "error.h"

namespace n3::linux::epoll {
class epoll_executor {
    epoll_ctx epoll;
    bool active;

    n3::PageBuffer read_buffer;

public:
    epoll_executor();

    //TODO: Need to have a proper handle type to pass in here or template off of
    [[nodiscard]] auto add(const int fd) noexcept -> const std::expected<void, error::ErrorCode>;

    /*
     * TODO: Need a few more functions
     *  - Run (main loop invocation, may want a run_once split off)
     *  - Registering read/write events and callbacks (Done in an init function, or per-call?)
     *  - Memory buffer to handle data reads that we can return to the user
     */
    void run();
};
}; // namespace n3::linux::epoll

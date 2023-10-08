#pragma once

#include <expected>

#include "epoll.h"
#include "error.h"

namespace n3 { namespace epoll {
    class epoll_executor {
        epoll_ctx epoll;

    public:
        epoll_executor();

        //TODO: Need to have a proper handle type to pass in here or template off of
        [[nodiscard]] auto add(const int fd) noexcept -> const std::expected<void, error::code>;

        /*
         * TODO: Need a few more functions
         *  - Run (main loop invocation, may want a run_once split off)
         *  - Registering read/write events and callbacks (Done in an init function, or per-call?)
         *  - Memory buffer to handle data reads that we can return to the user
         *  - Need a main TCP socket wrapper to properly use the add() function (IP/port translation, DNS lookup, etc)
         */
    };
}}; // namespace n3::epoll

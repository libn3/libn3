#pragma once

#include <array>
#include <memory>
#include <sys/epoll.h>
#include <vector>

namespace epoll {

    class epoll_handle {
        int efd;

    public:
        epoll_handle();
        ~epoll_handle() noexcept;
    };

    class epoll_ctx {
        static constexpr size_t EVENT_BUFFER_SIZE = 32768;

        epoll_handle efd;
        std::vector<int> descriptors;
        std::array<struct epoll_event, EVENT_BUFFER_SIZE> events;

    public:
        epoll_ctx();
    };

} //namespace epoll

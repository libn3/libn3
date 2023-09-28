#pragma once

#include <array>
#include <expected>
#include <memory>
#include <sys/epoll.h>
#include <vector>

#include "error.h"

namespace epoll {

class epoll_handle {
public:
    const int efd;

    epoll_handle();
    ~epoll_handle() noexcept;

    epoll_handle(const epoll_handle&) = delete;
    epoll_handle(epoll_handle&&) = default;

    epoll_handle& operator=(const epoll_handle&) = delete;
    epoll_handle& operator=(epoll_handle&&) = default;
};

class epoll_ctx {
    static constexpr size_t EVENT_BUFFER_SIZE = 32768;

    epoll_handle efd;
    std::vector<int> descriptors;
    std::array<struct epoll_event, EVENT_BUFFER_SIZE> events;

public:
    epoll_ctx();
    epoll_ctx(const epoll_ctx&) = default;
    epoll_ctx(epoll_ctx&&) = default;

    const std::expected<void, error::code> add(const int fd) noexcept;
};

} //namespace epoll

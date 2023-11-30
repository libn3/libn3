#pragma once

#include <array>
#include <chrono>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <sys/epoll.h>
#include <vector>

#include "error.h"

namespace n3::linux::epoll {

class epoll_handle {
public:
    const int efd;

    epoll_handle();
    ~epoll_handle() noexcept;

    epoll_handle(const epoll_handle&) = delete;
    epoll_handle(epoll_handle&&) noexcept = default;

    epoll_handle& operator=(const epoll_handle&) = delete;
    epoll_handle& operator=(epoll_handle&&) = default;
};

class epoll_ctx {
    static constexpr size_t EVENT_BUFFER_SIZE = 32768;

    const epoll_handle efd;
    std::vector<int> descriptors;
    std::array<struct epoll_event, EVENT_BUFFER_SIZE> events;

public:
    epoll_ctx();
    epoll_ctx(const epoll_ctx&) = default;
    epoll_ctx(epoll_ctx&&) = default;

    epoll_ctx& operator=(const epoll_ctx&) noexcept = default;
    epoll_ctx& operator=(epoll_ctx&&) noexcept = default;

    //TODO: Strongly type the fd instead of using int
    [[nodiscard]] auto add(const int fd) noexcept -> const std::expected<void, error::ErrorCode>;
    [[nodiscard]] auto remove(const int fd) noexcept -> const std::expected<void, error::ErrorCode>;
    //TODO: Wrap the raw epoll_event struct into my own type?
    [[nodiscard]] auto wait(const std::optional<std::chrono::milliseconds>& timeout_ms) noexcept
            -> const std::expected<std::span<struct epoll_event>, error::ErrorCode>;
};

} // namespace n3::linux::epoll

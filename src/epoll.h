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
#include "handle.h"

namespace n3::linux::epoll {

/*
 * TODO: Is this even useful anymore?
 * Can probably get away with hiding it in the epoll_ctx and using the OwnedHandle type
 * for the RAII cleanup aspects
 */
class epoll_handle {
public:
    const Handle efd;

    epoll_handle();
    ~epoll_handle() noexcept;
};

class epoll_ctx {
    static constexpr size_t EVENT_BUFFER_SIZE = 32768;

    const epoll_handle efd;
    std::array<::epoll_event, EVENT_BUFFER_SIZE> events;

public:
    epoll_ctx();
    epoll_ctx(const epoll_ctx&) noexcept = default;
    epoll_ctx(epoll_ctx&&) noexcept = default;

    epoll_ctx& operator=(const epoll_ctx&) noexcept = default;
    epoll_ctx& operator=(epoll_ctx&&) noexcept = default;

    [[nodiscard]] auto add(const Handle fd) noexcept -> const std::expected<void, error::ErrorCode>;
    [[nodiscard]] auto remove(const Handle fd) noexcept
            -> const std::expected<void, error::ErrorCode>;
    [[nodiscard]] auto wait(const std::optional<const std::chrono::milliseconds>& timeout_ms
            = std::nullopt) noexcept
            -> const std::expected<const std::span<const ::epoll_event>, error::ErrorCode>;
};

} // namespace n3::linux::epoll

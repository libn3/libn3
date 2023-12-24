#include <cassert>
#include <cerrno>
#include <chrono>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <sys/epoll.h>
#include <system_error>
#include <unistd.h>
#include <utility>
#include <vector>

#include "epoll.h"

#include "error.h"
#include "handle.h"

namespace n3::linux::epoll {

epoll_handle::epoll_handle() : efd(epoll_create1(EPOLL_CLOEXEC)) {
    if (this->efd == -1) {
        throw error::get_error_code_from_errno(errno);
    }
}

epoll_handle::~epoll_handle() noexcept {
    //No good way to handle error returns, maybe an eventual "cleanup error callback function?"
    close(this->efd);
}

epoll_ctx::epoll_ctx() : efd{}, descriptors{}, events{} {
    descriptors.reserve(epoll_ctx::EVENT_BUFFER_SIZE);
}

[[nodiscard]] auto epoll_ctx::add(Handle fd) noexcept
        -> const std::expected<void, error::ErrorCode> {
    static constexpr auto EVENT_MASK = (EPOLLIN | EPOLLOUT | EPOLLET | EPOLLEXCLUSIVE);
    ::epoll_event event{
            .events = EVENT_MASK,
            .data{.ptr = this},
    };

    const auto ret = epoll_ctl(this->efd.efd, EPOLL_CTL_ADD, fd, &event);
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    this->descriptors.emplace_back(fd);
    return {};
}

[[nodiscard]] auto epoll_ctx::remove(Handle fd) noexcept
        -> const std::expected<void, error::ErrorCode> {
    const auto ret = epoll_ctl(this->efd.efd, EPOLL_CTL_DEL, fd, nullptr);
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    this->descriptors.emplace_back(fd);
    return {};
}

[[nodiscard]] auto epoll_ctx::wait(
        const std::optional<const std::chrono::milliseconds>& timeout_ms) noexcept
        -> const std::expected<std::span<const ::epoll_event>, error::ErrorCode> {
    //Zero will block forever if the timeout optional is empty
    const int epoll_timeout = timeout_ms.value_or(std::chrono::milliseconds{0}).count();
    const auto ret
            = epoll_wait(this->efd.efd, this->events.data(), this->events.size(), epoll_timeout);
    if (ret == -1) {
        if (errno == EINTR) {
            return std::unexpected(error::get_error_code_from_errno(ETIMEDOUT));
        }
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    if (ret == 0) {
        //Timeout
        return std::unexpected(error::get_error_code_from_errno(ETIMEDOUT));
    }
    assert(ret > 0 && static_cast<decltype(this->events.size())>(ret) <= this->events.size());

    return std::span<const ::epoll_event>{this->events.data(), static_cast<size_t>(ret)};
}

} // namespace n3::linux::epoll

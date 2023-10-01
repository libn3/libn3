#include <cassert>
#include <cerrno>
#include <memory>
#include <span>
#include <sys/epoll.h>
#include <system_error>
#include <expected>
#include <unistd.h>
#include <utility>
#include <vector>

#include "epoll.h"

#include "error.h"

namespace n3 { namespace epoll {

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
        descriptors.reserve(32768);
    }

    [[nodiscard]] auto epoll_ctx::add(const int fd) noexcept
            -> const std::expected<void, error::code> {
        static constexpr auto EVENT_MASK = (EPOLLIN | EPOLLOUT | EPOLLET | EPOLLEXCLUSIVE);
        struct epoll_event event {
            .events = EVENT_MASK, .data{.ptr = this},
        };

        const auto ret = epoll_ctl(this->efd.efd, EPOLL_CTL_ADD, fd, &event);
        if (ret == -1) {
            return std::unexpected(error::get_error_code_from_errno(errno));
        }
        this->descriptors.emplace_back(fd);
        return {};
    }

    [[nodiscard]] auto epoll_ctx::remove(const int fd) noexcept
            -> const std::expected<void, error::code> {
        const auto ret = epoll_ctl(this->efd.efd, EPOLL_CTL_DEL, fd, nullptr);
        if (ret == -1) {
            return std::unexpected(error::get_error_code_from_errno(errno));
        }
        this->descriptors.emplace_back(fd);
        return {};
    }

    [[nodiscard]] auto epoll_ctx::wait() noexcept
            -> const std::expected<std::span<struct epoll_event>, error::code> {
        //TODO: Currently no timeout is set
        const auto ret = epoll_wait(this->efd.efd, this->events.data(), this->events.size(), 0);
        if (ret == -1) {
            return std::unexpected(error::get_error_code_from_errno(errno));
        }
        if (ret == 0) {
            //Timeout
            return std::unexpected(error::get_error_code_from_errno(ETIMEDOUT));
        }
        assert(ret > 0 && static_cast<decltype(this->events.size())>(ret) <= this->events.size());

        return std::span<struct epoll_event>{this->events.data(), static_cast<size_t>(ret)};
    }

}} // namespace n3::epoll

#include <cerrno>
#include <memory>
#include <sys/epoll.h>
#include <system_error>
#include <unistd.h>
#include <utility>
#include <vector>

#include "epoll.h"

#include "error.h"

namespace epoll {

epoll_handle::epoll_handle() : efd(epoll_create1(EPOLL_CLOEXEC)) {
    if (this->efd == -1) {
        throw std::system_error(errno, std::generic_category(), "epoll_create1");
    }
}

epoll_handle::~epoll_handle() noexcept {
    //No good way to handle error returns, maybe an eventual "cleanup error callback function?"
    close(this->efd);
}

epoll_ctx::epoll_ctx() : efd{}, descriptors{}, events{} {
    descriptors.reserve(32768);
}

const std::expected<void, error::code> epoll_ctx::add(const int fd) noexcept {
    struct epoll_event event {
        .events = (EPOLLIN | EPOLLOUT | EPOLLET), .data{.ptr = nullptr},
    };

    const auto ret = epoll_ctl(this->efd.efd, EPOLL_CTL_ADD, fd, &event);
    if (ret == -1) {
        return std::unexpected(error::code::TEST);
    }
    return {};
}

} //namespace epoll

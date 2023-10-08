#include <algorithm>
#include <array>

#include "epoll_executor.h"

#include "epoll.h"

namespace n3 { namespace epoll {
    epoll_executor::epoll_executor() : epoll{} {
    }

    //TODO: Need to have a proper handle type to pass in here or template off of
    [[nodiscard]] auto epoll_executor::add(const int fd) noexcept
            -> const std::expected<void, error::code> {
        return this->epoll.add(fd);
    }
}}; // namespace n3::epoll

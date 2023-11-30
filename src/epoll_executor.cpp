#include <algorithm>
#include <array>

#include "epoll_executor.h"

#include "epoll.h"
#include "error.h"

namespace n3::linux::epoll {

epoll_executor::epoll_executor() : epoll{} {
}

//TODO: Need to have a proper handle type to pass in here or template off of
[[nodiscard]] auto epoll_executor::add(const int fd) noexcept
        -> const std::expected<void, error::ErrorCode> {
    return this->epoll.add(fd);
}

void epoll_executor::run() {
    while (this->active) {
        const auto events = this->epoll.wait(std::nullopt);
        if (!events.has_value()) {
            const auto err = events.error();
            if (err == error::posix_error{ETIMEDOUT}) {
                continue;
            }
            //TODO: How to handle general epoll errors?
            continue;
        }
        assert(events.has_value());
        //TODO: Where and how am I managing the epoll return values?
    }
    return;
}

}; // namespace n3::linux::epoll

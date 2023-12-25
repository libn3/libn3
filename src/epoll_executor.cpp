#include <algorithm>
#include <array>

#include "epoll_executor.h"

#include "epoll.h"
#include "error.h"
#include "handle.h"

namespace n3::linux::epoll {

epoll_executor::epoll_executor() : epoll{} {
}

[[nodiscard]] auto epoll_executor::add(Handle fd) noexcept
        -> const std::expected<void, error::ErrorCode> {
    return this->epoll.add(fd);
}

[[nodiscard]] auto epoll_executor::remove(Handle fd) noexcept
        -> const std::expected<void, error::ErrorCode> {
    return this->epoll.remove(fd);
}

void epoll_executor::run_once() {
    const auto events = this->epoll.wait();
    if (!events.has_value()) {
        const auto err = events.error();
        if (err == error::posix_error{ETIMEDOUT}) {
            return;
        }
        //TODO: How to handle general epoll errors?
        return;
    }
    assert(events.has_value());
    //TODO: Where and how am I managing the epoll return values?
}

void epoll_executor::run() {
    while (this->active) {
        this->run_once();
    }
}

}; // namespace n3::linux::epoll

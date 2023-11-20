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

#include "runtime.h"

#include "epoll.h"
#include "error.h"

namespace n3::runtime {

runtime_st::runtime_st() noexcept : epoll{}, active{false} {
}

void runtime_st::run() {
    while (this->active) {
        const auto events = this->epoll.wait(std::nullopt);
        if (!events.has_value()) {
            const auto err = events.error();
            if (err == ETIMEDOUT) {
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

} // namespace n3::runtime

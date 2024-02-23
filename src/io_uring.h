#pragma once

#include <liburing.h>

#include "buffer.h"
#include "error.h"
#include "handle.h"
#include "ownership.h"

namespace n3::linux::io_uring {

class io_uring_handle {
    MoveOnly<::io_uring> ring;

public:
    io_uring_handle();
    ~io_uring_handle();

    constexpr const ::io_uring *get() const noexcept {
        return &*ring;
    }
    constexpr ::io_uring *get() noexcept {
        return &*ring;
    }
};

[[nodiscard]] std::optional<std::reference_wrapper<::io_uring_sqe>> get_sqe(
        io_uring_handle& handle) noexcept;

}; // namespace n3::linux::io_uring

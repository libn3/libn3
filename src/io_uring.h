#pragma once

#include <liburing.h>

#include "buffer.h"
#include "error.h"
#include "handle.h"

namespace n3::linux::io_uring {

class io_uring_handle {
    std::unique_ptr<struct io_uring> ring;

public:
    io_uring_handle();
    ~io_uring_handle();

    constexpr io_uring_handle(const io_uring_handle&) = delete;
    constexpr io_uring_handle(io_uring_handle&& other) = default;

    constexpr io_uring_handle& operator=(const io_uring_handle&) = delete;
    constexpr io_uring_handle& operator=(io_uring_handle&& other) = default;
};

}; // namespace n3::linux::io_uring

#pragma once

#include <liburing.h>

#include "buffer.h"
#include "error.h"
#include "handle.h"
#include "ownership.h"

namespace n3::linux::io_uring {

class io_uring_handle {
    MoveOnly<struct io_uring> ring;

public:
    io_uring_handle();
    ~io_uring_handle();
};

}; // namespace n3::linux::io_uring

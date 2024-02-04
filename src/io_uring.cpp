#include <liburing.h>

#include "io_uring.h"

namespace n3::linux::io_uring {

static constexpr unsigned int IO_URING_SQ_LEN = 1024;
static constexpr unsigned int IO_URING_CQ_LEN = 32768;

io_uring_handle::io_uring_handle() : ring{} {
    io_uring_params params;
    memset(&params, 0, sizeof(params));

    params.sq_entries = IO_URING_SQ_LEN;
    params.cq_entries = IO_URING_CQ_LEN;
    params.flags = (IORING_SETUP_SQPOLL | IORING_SETUP_CQSIZE | IORING_SETUP_CLAMP
            | IORING_SETUP_SINGLE_ISSUER | IORING_SETUP_DEFER_TASKRUN);

    int ret = io_uring_queue_init_params(IO_URING_SQ_LEN, &*this->ring, &params);
    if (ret != 0) {
        throw error::get_error_code_from_errno(-ret);
    }
    ret = io_uring_register_ring_fd(&*this->ring);
    if (ret != 0) {
        throw error::get_error_code_from_errno(-ret);
    }
}

io_uring_handle::~io_uring_handle() {
    if (this->ring) {
        io_uring_queue_exit(&*this->ring);
    }
}

} // namespace n3::linux::io_uring

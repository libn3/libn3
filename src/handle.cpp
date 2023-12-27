#include <unistd.h>

#include "handle.h"

namespace n3 {

OwnedHandle::OwnedHandle(const int fd_arg) noexcept : fd{fd_arg} {
}

OwnedHandle::~OwnedHandle() {
    //No good way to handle error returns, maybe an eventual "cleanup error callback function?"
    close(this->fd);
}

} // namespace n3

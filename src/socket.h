#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <climits>
#include <cstdint>
#include <expected>
#include <fcntl.h>
#include <linux/filter.h>
#include <linux/icmp.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/socket.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <memory>
#include <netdb.h>
#include <span>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "error.h"
#include "syscalls.h"

namespace n3::net::linux {

template<typename T>
class socket {
public:
    //Keep this type an interface instead of a full virtual base class
    virtual ~socket() = 0;

    std::expected<void, error::code> listen(const int sock, const int backlog) noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::listen)>) {
            return static_cast<T const *>(this)->listen(sock, backlog);
        }
        return n3::linux::listen(sock, backlog);
    }

    //std::expected<size_t, error::code> send(const std::span<std::byte> buf) const noexcept {
    //    const auto buf_vec = std::span<const std::span<std::byte>, 1>{std::addressof(buf), 1zu};
    //    return write_vectored(std::move(buf_vec));
    //}
    //std::expected<size_t, error::code> send(
    //        const std::span<std::span<std::byte>> bufs) const noexcept {
    //    //TODO: Checked at runtime, might be a better way of handling this?
    //    if (bufs.size() > IOV_MAX) {
    //        return std::unexpected(error::get_error_code_from_errno(EINVAL));
    //    }
    //    return write_vectored(bufs);
    //}
};

} // namespace n3::net::linux

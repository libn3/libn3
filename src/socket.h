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

namespace n3 { namespace net { namespace linux {

    class socket {
        //std::expected<size_t, error::code> write_vectored(
        //        const std::span<const std::span<std::byte>> bufs) const noexcept {
        //    //Validate assumption about the length of the caller
        //    assert(bufs.size() <= IOV_MAX);

        //    std::array<struct iovec, IOV_MAX> converted_bufs;
        //    //Convert spans to iovecs by copying to a stack buffer
        //    //There's probably a way of converting things without a copy...
        //    std::transform(bufs.cbegin(), bufs.cend(), converted_bufs.begin(), [](const auto vec) {
        //        return iovec{.iov_base = vec.data(), .iov_len = vec.size_bytes()};
        //    });

        //    const auto ret = writev(this->sock, converted_bufs.data(), bufs.size());
        //    if (ret == -1) {
        //        return std::unexpected(error::get_error_code_from_errno(errno));
        //    }
        //    assert(ret >= 0);
        //    return ret;
        //}

    public:
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

}}} // namespace n3::net::linux

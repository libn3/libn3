#pragma once

#include <expected>
#include <span>
#include <sys/socket.h>

#include "address.h"
#include "buffer.h"
#include "error.h"

namespace n3::linux {
//TODO: Probably need to enum and error check all the different int args to prevent misuse

[[nodiscard]] constexpr size_t get_sockopt_size(const int level, const int optname) noexcept;

std::expected<void, error::code> setsockopt(
        const int sock, const int level, const int option, const RefBuffer option_value) noexcept;

std::expected<std::span<std::byte>, error::code> getsockopt(
        const int sock, const int level, const int option, const RefBuffer option_buf) noexcept;

//TODO: Convert to use RefMultiBuffer
std::expected<size_t, error::code> readv(
        const int fd, std::span<std::span<std::byte>> bufs) noexcept;
std::expected<size_t, error::code> writev(
        const int fd, std::span<std::span<std::byte>> bufs) noexcept;

std::expected<size_t, error::code> send(
        const int sock, const RefBuffer buf, const int flags) noexcept;

template<n3::net::AddressType T>
std::expected<size_t, error::code> sendto(
        const int sock, const RefBuffer buf, const T& addr, const int flags) noexcept {
    const auto raw_addr = addr.to_sockaddr();
    const auto ret = ::sendto(sock,
            buf.as_span().data(),
            buf.as_span().size_bytes(),
            flags,
            dynamic_cast<::sockaddr *>(std::addressof(raw_addr)),
            sizeof(raw_addr));
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    return ret;
}

std::expected<size_t, error::code> sendmsg(
        const int sock, const ::msghdr& msg, const int flags) noexcept;

std::expected<size_t, error::code> recv(const int sock, RefBuffer buf, const int flags) noexcept;

std::expected<std::pair<size_t, std::variant<n3::net::v4::address, n3::net::v6::address>>,
        error::code>
        recvfrom(const int sock, RefBuffer buf, const int flags) noexcept;

std::expected<std::pair<size_t, ::msghdr>, error::code> recvmsg(
        const int sock, const int flags) noexcept;

std::expected<long, error::code> sysconf(const int name) noexcept;

template<n3::net::AddressType T>
std::expected<void, error::code> connect(const int sock, const T& addr) noexcept {
    const auto raw_addr = addr.to_sockaddr();
    const auto ret = ::connect(
            sock, dynamic_cast<::sockaddr *>(std::addressof(raw_addr)), sizeof(raw_addr));
    if (ret == -1) {
        //TODO: Probably want to move this one level of abstraction up, and keep the syscall wrapper simple
        if (errno == EINPROGRESS) {
            //Nonblocking connection initiated as expected
            //TODO: Unix sockets use EAGAIN, do we need a template specialization just for them?
            return {};
        }
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    return {};
}

template<n3::net::AddressType T>
std::expected<void, error::code> bind(const int sock, const T& addr) noexcept {
    const auto raw_addr = addr.to_sockaddr();
    const auto ret
            = ::bind(sock, dynamic_cast<::sockaddr *>(std::addressof(raw_addr)), sizeof(raw_addr));
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    return {};
}

std::expected<void, error::code> listen(const int sock, const int backlog) noexcept;

std::expected<std::pair<int, std::variant<n3::net::v4::address, n3::net::v6::address>>, error::code>
        accept(const int sock) noexcept;

} // namespace n3::linux

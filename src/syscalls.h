#pragma once

#include <expected>
#include <span>

#include "buffer.h"
#include "error.h"

namespace n3 { namespace linux {
    //TODO: Probably need to enum and error check all the different int args to prevent misuse

    [[nodiscard]] constexpr size_t get_sockopt_size(const int level, const int optname) noexcept;

    std::expected<void, error::code> setsockopt(const int sock,
            const int level,
            const int option,
            const RefBuffer option_value) noexcept;

    std::expected<std::span<std::byte>, error::code> getsockopt(
            const int sock, const int level, const int option, const RefBuffer option_buf) noexcept;

    std::expected<size_t, error::code> readv(
            const int fd, std::span<std::span<std::byte>> bufs) noexcept;
    std::expected<size_t, error::code> writev(
            const int fd, std::span<std::span<std::byte>> bufs) noexcept;

    std::expected<size_t, error::code> send(
            const int sock, const RefBuffer buf, const int flags) noexcept;
    //std::expected<size_t, error::code> sendto(const int sockfd, const RefBuffer, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
    //std::expected<size_t, error::code> sendmsg(const int sockfd, const struct msghdr *msg, int flags);
}} // namespace n3::linux

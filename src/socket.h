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

    std::expected<void, error::code> listen(const int sock) const noexcept {
        //Values above SOMAXCONN are truncated
        //As of linux 5.4, default is 4096, older versions use 128
        static constexpr int backlog = SOMAXCONN;

        if constexpr (std::is_member_function_pointer_v<decltype(&T::listen)>) {
            return static_cast<T const *>(this)->listen(sock);
        }
        return n3::linux::listen(sock, backlog);
    }

    std::expected<size_t, error::code> send(
            const int sock, const RefBuffer buf, const int flags) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::send)>) {
            return static_cast<T const *>(this)->send(sock, buf, flags);
        }
        return n3::linux::send(sock, buf, flags);
    }

    std::expected<size_t, error::code> recv(
            const int sock, RefBuffer buf, const int flags) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::recv)>) {
            return static_cast<T const *>(this)->recv(sock, buf, flags);
        }
        return n3::linux::recv(sock, buf, flags);
    }

    template<n3::net::AddressType U>
    std::expected<void, error::code> bind(const int sock, const U& addr) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::template bind<U>)>) {
            return static_cast<T const *>(this)->template bind<U>(sock, addr);
        }
        return n3::linux::bind<U>(sock, addr);
    }

    template<n3::net::AddressType U>
    std::expected<void, error::code> connect(const int sock, const T& addr) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::template connect<U>)>) {
            return static_cast<T const *>(this)->template connect<U>(sock, addr);
        }
        return n3::linux::connect<U>(sock, addr);
    }

    std::expected<std::pair<int, std::variant<n3::net::v4::address, n3::net::v6::address>>,
            error::code>
            accept(const int sock) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::accept)>) {
            return static_cast<T const *>(this)->accept(sock);
        }
        return n3::linux::accept(sock);
    }

    //TODO: Type check/verify sizes against the size of the buffer
    std::expected<void, error::code> setsockopt(const int sock,
            const int level,
            const int option,
            const RefBuffer option_value) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::setsockopt)>) {
            return static_cast<T const *>(this)->setsockopt(sock, level, option, option_value);
        }
        return n3::linux::setsockopt(sock, level, option, option_value);
    }

    //TODO: Type check/verify sizes against the size of the buffer
    std::expected<std::span<std::byte>, error::code> getsockopt(const int sock,
            const int level,
            const int option,
            const RefBuffer option_buf) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::getsockopt)>) {
            return static_cast<T const *>(this)->getsockopt(sock, level, option, option_buf);
        }
        return n3::linux::getsockopt(sock, level, option, option_buf);
    }
};

} // namespace n3::net::linux

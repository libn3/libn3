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
#include "runtime.h"
#include "syscalls.h"

namespace n3::net::linux {

/*
 * TODO: Most of these functions need the template magic to create a callback object with arbitrary args
 * Things like recv() will be async, so we need the callback semantics/logic in order to actually use
 * the event loop we're designing the library for
 */

template<typename T>
class socket {
public:
    //Keep this type an interface instead of a full virtual base class
    virtual ~socket() = 0;

    std::expected<void, error::ErrorCode> listen(const int sock) const noexcept {
        //Values above SOMAXCONN are truncated
        //As of linux 5.4, default is 4096, older versions use 128
        static constexpr int backlog = SOMAXCONN;

        if constexpr (std::is_member_function_pointer_v<decltype(&T::listen)>) {
            return static_cast<T const *>(this)->listen(sock);
        }
        return n3::linux::listen(sock, backlog).transform_error(&error::get_error_code_from_errno);
    }

    //TODO: Probably needs a callback invocable concept that replicates the bind_front arg stacking
    //TODO: Verify that you can construct a callback object with the template types/args
    //TODO: Do I need to make the noexcept qualifier here conditional on something?
    template<typename F, typename... Args>
        requires std::invocable<F, std::expected<size_t, error::ErrorCode>, Args...>
    void send(const int sock, const RefBuffer buf, const int flags, F&& cb_func, Args&&...cb_args)
            const noexcept {
        //TODO: Verify that any child types that override match the same signature
        if constexpr (std::is_member_function_pointer_v<decltype(&T::send)>) {
            return static_cast<T const *>(this)->send(sock, buf, flags);
        }
        auto&& ret = n3::linux::send(sock, buf, flags);

        //TODO: I hate this check, requires error code design change to be more user friendly
        if (!ret.has_value() && ret.error() == EAGAIN) {
            //TODO: Save the callback in the event loop somehow for future use when complete
            [[maybe_unused]] n3::runtime::callback<std::expected<size_t, error::ErrorCode>> cb{
                    std::forward<F&&>(cb_func), std::forward<Args&&...>(cb_args...)};
            return;
        }
        //Either a successful return code or abnormal error, either way, call the callback now
        std::invoke(cb_func, ret.transform_error(&error::get_error_code_from_errno), cb_args...);
    }

    //TODO: What is the plan with socket object syscall wrapper return types?
    //TODO: Is everything here just returning void and always using a callback?
    std::expected<size_t, error::ErrorCode> recv(
            const int sock, RefBuffer buf, const int flags) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::recv)>) {
            return static_cast<T const *>(this)->recv(sock, buf, flags);
        }
        return n3::linux::recv(sock, buf, flags).transform_error(&error::get_error_code_from_errno);
    }

    template<n3::net::AddressType U>
    std::expected<void, error::ErrorCode> bind(const int sock, const U& addr) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::template bind<U>)>) {
            return static_cast<T const *>(this)->template bind<U>(sock, addr);
        }
        return n3::linux::bind<U>(sock, addr).transform_error(&error::get_error_code_from_errno);
    }

    template<n3::net::AddressType U>
    std::expected<void, error::ErrorCode> connect(const int sock, const U& addr) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::template connect<U>)>) {
            return static_cast<T const *>(this)->template connect<U>(sock, addr);
        }
        return n3::linux::connect<U>(sock, addr);
    }

    std::expected<std::pair<int, std::variant<n3::net::v4::address, n3::net::v6::address>>,
            error::ErrorCode>
            accept(const int sock) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::accept)>) {
            return static_cast<T const *>(this)->accept(sock);
        }
        return n3::linux::accept(sock).transform_error(&error::get_error_code_from_errno);
    }

    //TODO: Type check/verify sizes against the size of the buffer
    std::expected<void, error::ErrorCode> setsockopt(const int sock,
            const int level,
            const int option,
            const RefBuffer option_value) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::setsockopt)>) {
            return static_cast<T const *>(this)->setsockopt(sock, level, option, option_value);
        }
        return n3::linux::setsockopt(sock, level, option, option_value);
    }

    //TODO: Type check/verify sizes against the size of the buffer
    std::expected<std::span<std::byte>, error::ErrorCode> getsockopt(const int sock,
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

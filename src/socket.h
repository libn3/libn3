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

/*
 * TODO: A lot of functions here take an int fd handle, but real wrapper types won't be doing that
 * Do I need to restructure things to fetch it properly or change the whole state/interface type setup?
 * If a socket interface needs to store data, then you're just doing OOP like normal with all the issues
 * I want any inheritance to be CRTP interfaces with zero state, or pursue something else entirely
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
        return n3::linux::listen(sock, backlog);
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
        /*
         * TODO: This is actually wrong given the current "read/write event queue per socket" design
         * Attempting an immediate send would reorder data since there may be pending writes already
         * queued up that need to be sent first
         * So instead of calling the syscall here directly, this should probably call some higher
         * level wrapper that flushes the pending write queue
         */
        auto&& ret = n3::linux::send(sock, buf, flags);

        //Call the callback straight away if things immediately succeed
        if (ret.has_value()) {
            std::invoke(cb_func, ret, cb_args...);
            return;
        }

        assert(!ret.has_value());

        if (ret.error() == error::posix_error{EAGAIN}) {
            //TODO: Save the callback in the event loop somehow for future use when complete
            /*
             * TODO: Before I can figure out how to store async context data,
             * I need to figure out how to handle multi-read/write calls
             * What happens if someone calls TcpSocket::read 3 times in a row?
             * As well as how that differs between single syscall wrapper functions, and composed
             * multi-syscall functions (read_once() vs read() vs read_exact())
             *
             * Also needs to generalize to stuff like bind(), connect(), and accept(), not just
             * read and write
             *
             * Do we:
             *  - Forbid it entirely (UB)
             *  - Check for it (return an error)
             *  - Replace previous slot (dedicated read/write handler is registered with IO loop)
             *  - Queue them up back-to-back
             *       - on a smart read() wrapper, does that mean a full EAGAIN exhaustion per call?
             *  - Try and cleverly merge them under the hood
             *       - growing buffer,
             *       - span merging
             *       - transparently switch to vectored read/write syscalls (multiple refbuffers queued up)
             * I WANT TO BE CLEVER ABOUT IT!!!
             */
            [[maybe_unused]] n3::runtime::callback<std::expected<size_t, error::ErrorCode>> cb{
                    std::forward<F&&>(cb_func), std::forward<Args&&...>(cb_args...)};
            return;
        }
        //Error value with no special meaning, call the callback immediately
        std::invoke(cb_func, ret, cb_args...);
    }

    //TODO: What is the plan with socket object syscall wrapper return types?
    //TODO: Is everything here just returning void and always using a callback?
    std::expected<size_t, error::ErrorCode> recv(
            const int sock, RefBuffer buf, const int flags) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::recv)>) {
            return static_cast<T const *>(this)->recv(sock, buf, flags);
        }
        return n3::linux::recv(sock, buf, flags);
    }

    template<n3::net::AddressType U>
    std::expected<void, error::ErrorCode> bind(const int sock, const U& addr) const noexcept {
        if constexpr (std::is_member_function_pointer_v<decltype(&T::template bind<U>)>) {
            return static_cast<T const *>(this)->template bind<U>(sock, addr);
        }
        return n3::linux::bind<U>(sock, addr);
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
        return n3::linux::accept(sock);
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

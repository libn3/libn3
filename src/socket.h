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

//TODO: THIS DOESN'T HAVE TO BE A CRTP CLASS IT CAN JUST USE TEMPLATES AND SPECIALIZATION!!!

template<typename T>
class socket {
public:
    //Keep this type an interface instead of a full virtual base class
    //virtual ~socket() = 0;

    std::expected<void, error::ErrorCode> listen(const int sock) const noexcept {
        //Values above SOMAXCONN are truncated
        //As of linux 5.4, default is 4096, older versions use 128
        static constexpr int backlog = SOMAXCONN;

        if constexpr (std::is_member_function_pointer_v<decltype(&T::listen)>) {
            return static_cast<T const *>(this)->listen(sock);
        }
        return n3::linux::listen(sock, backlog);
    }

    //const T& sock;
    //const int fd = sock.native_handle();
    //send();
    //TODO: Figure out how the whole "save callback to write queue" thing works, is that an optional return value?
    /*
     * TODO: Is there a way to "sink" the callback addition work to never require a return value?
     * I'm imaginging it would require something on the general IO context loop executor thing
     * Idea being, you register/add a new callback to the read/write queue of a native handle,
     * which is all stored in a single massive hashmap/tree for actual usage
     *
     * The buffer coalescing I want can be handled in making a read/write buffer queue wrapper type
     *
     * Was also thinking just not doing the syscall work in the actual function here,
     * only registering what you need, and returning
     * Idea there being that you can bulk dispatch and/or simplify things by lazily actioning the command
     *
     * For epoll, likely limited value unless there's some syscall magic I can't think of right now
     *
     * For io_uring however, this can be a really big win because you'd have more data/context to be
     * able to do things like chaining syscalls on the same fd, doing bulk insertions to the
     * submission queue or other similar tricks
     *
     * Not to mention the fact that you'd benefit from shared memory handling a bit
     * If everything lives in a single important lookup map, then that part of memory is always in cache
     * Whereas if every socket wrapper has its own queues, now you're running in a totally unique allocation
     * It basically consolidates the memory allocation and usage in a cache friendlier way
     *
     * You already have to pay _some_ kinda price even for eager execution because of the fact
     * that read/write queues per socket exist, so you need to flush old data before new to
     * not reorder things from the user's application
     * So if you're already having to push something to a queue for the socket prior to actually
     * doing things, what's one more queue push?
     *
     * Practically speaking, this would mean 2 kinda designs given the proactor/reactor differences
     * between epoll and io_uring
     *
     * For epoll, owning callback of the syscall and args pushed into a read/write interest queue,
     * and then flushed prior to going back to waiting on epoll
     * For io_uring and IOCP, trigger the syscall immediately and register the callback in the lookup
     * table for when the completion event is received
     *
     * Probably means that for the different executors, the difference will mainly be about how the
     * lookup and queues are stored/handled
     * Namely:
     *  - Epoll has a lookup of fds pointing to a pair of read/write queues
     *  - io_uring/IOCP has a lookup of completion events pointing to callbacks
     */

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
            [[maybe_unused]] n3::callback<std::expected<size_t, error::ErrorCode>> cb{
                    std::forward<F>(cb_func), std::forward<Args...>(cb_args...)};
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

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

#include "syscalls.h"

#include "address.h"
#include "error.h"

namespace n3::linux {

//TODO: Probably need to enum and error check this to prevent misuse on all possible ints
[[nodiscard]] constexpr size_t get_sockopt_size(const int level, const int optname) noexcept {
    switch (level) {
        case SOL_SOCKET:
            static_assert(SOL_SOCKET == IPPROTO_ICMP);
            return [=] [[nodiscard]] {
                switch (optname) {
                    case SO_ACCEPTCONN:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case SO_ATTACH_FILTER:
                        [[fallthrough]];
                    case SO_ATTACH_REUSEPORT_CBPF:
                        return sizeof(::sock_fprog);
                    case SO_ATTACH_BPF:
                        [[fallthrough]];
                    case SO_ATTACH_REUSEPORT_EBPF:
                        return sizeof(int);
                    case SO_BINDTODEVICE:
                        return static_cast<size_t>(IFNAMSIZ);
                    case SO_BROADCAST:
                        return sizeof(int);
                    case SO_DEBUG:
                        return sizeof(int);
                    case SO_DETACH_FILTER:
                        static_assert(SO_DETACH_FILTER == SO_DETACH_BPF);
                        return sizeof(int);
                    case SO_DOMAIN:
                        return sizeof(int);
                    case SO_ERROR:
                        return sizeof(int);
                    case SO_DONTROUTE:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case SO_INCOMING_CPU:
                        return sizeof(int);
                    case SO_INCOMING_NAPI_ID:
                        return sizeof(unsigned int);
                    case SO_KEEPALIVE:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case SO_LINGER:
                        return sizeof(::linger);
                    case SO_LOCK_FILTER:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case SO_MARK:
                        return sizeof(uint32_t);
                    case SO_OOBINLINE:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case SO_PASSCRED:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case SO_PEEK_OFF:
                        return sizeof(int);
                    case SO_PEERCRED:
                        return sizeof(::ucred);
                    case SO_PEERSEC:
                        return sizeof(int);
                    case SO_PRIORITY:
                        return sizeof(uint32_t);
                    case SO_PROTOCOL:
                        return sizeof(int);
                    case SO_RCVBUFFORCE:
                        [[fallthrough]];
                    case SO_RCVBUF:
                        return sizeof(int);
                    case SO_RCVLOWAT:
                        [[fallthrough]];
                    case SO_SNDLOWAT:
                        return sizeof(int);
                    case SO_RCVTIMEO:
                        [[fallthrough]];
                    case SO_SNDTIMEO:
                        return sizeof(::timeval);
                    case SO_REUSEADDR:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case SO_REUSEPORT:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case SO_RXQ_OVFL:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case SO_SNDBUFFORCE:
                        [[fallthrough]];
                    case SO_SNDBUF:
                        return sizeof(int);
                    case SO_TIMESTAMP:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case SO_TIMESTAMPNS:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case SO_TYPE:
                        return sizeof(int);
                    case SO_BUSY_POLL:
                        return sizeof(unsigned int);
                    default:
                        std::unreachable();
                }
            }();
        case IPPROTO_IP:
            return [=] [[nodiscard]] {
                switch (optname) {
                    case IP_ADD_MEMBERSHIP:
                        [[fallthrough]];
                    case IP_BLOCK_SOURCE:
                        [[fallthrough]];
                    case IP_DROP_MEMBERSHIP:
                        [[fallthrough]];
                    case IP_DROP_SOURCE_MEMBERSHIP:
                        return sizeof(::ip_mreqn);
                    case IP_ADD_SOURCE_MEMBERSHIP:
                        return sizeof(::ip_mreq_source);
                    case IP_BIND_ADDRESS_NO_PORT:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_FREEBIND:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_HDRINCL:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_LOCAL_PORT_RANGE:
                        return sizeof(uint32_t);
                    case IP_MSFILTER:
                        return sizeof(::ip_msfilter);
                    case IP_MTU:
                        return sizeof(int);
                    case IP_MTU_DISCOVER:
                        return sizeof(int);
                    case IP_MULTICAST_ALL:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_MULTICAST_IF:
                        return sizeof(::ip_mreqn);
                    case IP_MULTICAST_LOOP:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_MULTICAST_TTL:
                        return sizeof(int);
                    case IP_NODEFRAG:
                        return sizeof(int);
                    case IP_OPTIONS:
                        //Maximum option size for IPv4 packets
                        return 40uz;
                    case IP_PASSSEC:
                        //Not supported
                        std::unreachable();
                    case IP_PKTINFO:
                        return sizeof(::in_pktinfo);
                    case IP_RECVERR:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_RECVOPTS:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_RECVORIGDSTADDR:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_RECVTOS:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_RECVTTL:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_RETOPTS:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_ROUTER_ALERT:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_TOS:
                        //TOS is 1 byte
                        return 1uz;
                    case IP_TRANSPARENT:
                        //C style "int as bool" semantics
                        return sizeof(int);
                    case IP_TTL:
                        return sizeof(int);
                    case IP_UNBLOCK_SOURCE:
                        return sizeof(::ip_mreq_source);
                    case SO_PEERSEC:
                        //Not supported
                        std::unreachable();
                    default:
                        std::unreachable();
                }
            }();
        case IPPROTO_IPV6:
            return [=] [[nodiscard]] {
                switch (optname) {
                    case IPV6_ADDRFORM:
                        //Pointer to integer
                        return sizeof(int *);
                    case IPV6_ADD_MEMBERSHIP:
                        [[fallthrough]];
                    case IPV6_DROP_MEMBERSHIP:
                        return sizeof(::ipv6_mreq);
                    case IPV6_MTU:
                        //getsockopt returns an integer
                        //setsockopt expects a pointer to an integer
                        return ((sizeof(int) >= sizeof(int *)) ? sizeof(int) : sizeof(int *));
                    case IPV6_MTU_DISCOVER:
                        return sizeof(int);
                    case IPV6_MULTICAST_HOPS:
                        return sizeof(int *);
                    case IPV6_MULTICAST_IF:
                        return sizeof(int *);
                    case IPV6_MULTICAST_LOOP:
                        //Pointer to a bool as an int! How exciting!
                        return sizeof(int *);
                    case IPV6_RECVPKTINFO:
                        //Pointer to a bool as an int! How exciting!
                        return sizeof(int *);
                    case IPV6_RTHDR:
                        [[fallthrough]];
                    case IPV6_AUTHHDR:
                        [[fallthrough]];
                    case IPV6_DSTOPTS:
                        [[fallthrough]];
                    case IPV6_HOPOPTS:
                        [[fallthrough]];
                    case IPV6_HOPLIMIT:
                        //Pointer to a bool as an int! How exciting!
                        return sizeof(int *);
                    case IPV6_RECVERR:
                        //Pointer to a bool as an int! How exciting!
                        return sizeof(int *);
                    case IPV6_ROUTER_ALERT:
                        return sizeof(int *);
                    case IPV6_UNICAST_HOPS:
                        return sizeof(int *);
                    case IPV6_V6ONLY:
                        //Pointer to a bool as an int! How exciting!
                        return sizeof(int *);
                    default:
                        std::unreachable();
                }
            }();
        case IPPROTO_RAW:
            return [=] [[nodiscard]] {
                switch (optname) {
                    case ICMP_FILTER:
                        return sizeof(uint8_t);
                    default:
                        std::unreachable();
                }
            }();
        case IPPROTO_TCP:
            return [=] [[nodiscard]] {
                switch (optname) {
                    case TCP_CONGESTION:
                        /*
                             * Returns a string, with no mention of possible options or a max size
                             * Best I could find is the following list from wikipedia
                             *
                             * https://en.wikipedia.org/wiki/TCP_congestion_control#Linux_usage
                             *
                             * BIC is used by default in Linux kernels 2.6.8 through 2.6.18. (August 2004 – September 2006)
                             * CUBIC is used by default in Linux kernels since version 2.6.19. (November 2006)
                             * PRR is incorporated in Linux kernels to improve loss recovery since version 3.2. (January 2012)
                             * BBRv1 is incorporated in Linux kernels to enable model-based congestion control since version 4.9. (December 2016)
                             *
                             * So we'll go with 5 characters to match the longest known
                             */
                        return 5 * sizeof(unsigned char);
                    case TCP_INFO:
                        return sizeof(::tcp_info);
                    case TCP_USER_TIMEOUT:
                        return sizeof(unsigned int);
                    case TCP_CORK:
                    case TCP_DEFER_ACCEPT:
                    case TCP_KEEPCNT:
                    case TCP_KEEPIDLE:
                    case TCP_KEEPINTVL:
                    case TCP_LINGER2:
                    case TCP_MAXSEG:
                    case TCP_NODELAY:
                    case TCP_QUICKACK:
                    case TCP_SYNCNT:
                    case TCP_WINDOW_CLAMP:
                    case TCP_FASTOPEN:
                    case TCP_FASTOPEN_CONNECT:
                        return sizeof(int);
                    default:
                        std::unreachable();
                }
            }();
        case IPPROTO_UDP:
            return [=] [[nodiscard]] {
                switch (optname) {
                    case UDP_CORK:
                        return sizeof(int);
                    case UDP_SEGMENT:
                        return sizeof(int);
                    case UDP_GRO:
                        return sizeof(int);
                    default:
                        std::unreachable();
                }
            }();
        default:
            std::unreachable();
    }
    return 0;
}

std::expected<void, error::ErrorCode> setsockopt(
        const int sock, const int level, const int option, const RefBuffer option_value) noexcept {
    const auto ret = ::setsockopt(sock,
            level,
            option,
            option_value.as_span().data(),
            option_value.as_span().size_bytes());
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    return {};
}

std::expected<std::span<std::byte>, error::ErrorCode> getsockopt(
        const int sock, const int level, const int option, const RefBuffer option_buf) noexcept {
    //TODO: Set this up at compile time with templates instead of an application level check
    if (option_buf.as_span().size_bytes() < get_sockopt_size(level, option)) {
        return std::unexpected(error::get_error_code_from_errno(EINVAL));
    }

    socklen_t optlen = 0;
    const auto ret = ::getsockopt(sock, level, option, option_buf.as_span().data(), &optlen);
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    assert(optlen <= option_buf.as_span().size_bytes());
    return option_buf.as_span().first(optlen);
}

std::expected<size_t, error::ErrorCode> readv(
        const int fd, std::span<std::span<std::byte>> bufs) noexcept {
    //Validate assumption about the length of the caller
    //TODO: Set this up at compile time with templates instead of an application level check
    assert(bufs.size() <= IOV_MAX);

    std::array<::iovec, IOV_MAX> converted_bufs;
    //Convert spans to iovecs by copying to a stack buffer
    //There's probably a way of converting things without a copy...
    std::transform(bufs.cbegin(), bufs.cend(), converted_bufs.begin(), [](const auto vec) {
        return iovec{.iov_base = vec.data(), .iov_len = vec.size_bytes()};
    });

    const auto ret = readv(fd, converted_bufs.data(), bufs.size());
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    assert(ret >= 0);
    return ret;
}

std::expected<size_t, error::ErrorCode> writev(
        const int fd, std::span<std::span<std::byte>> bufs) noexcept {
    //Validate assumption about the length of the caller
    //TODO: Set this up at compile time with templates instead of an application level check
    assert(bufs.size() <= IOV_MAX);

    std::array<::iovec, IOV_MAX> converted_bufs;
    //Convert spans to iovecs by copying to a stack buffer
    //There's probably a way of converting things without a copy...
    std::transform(bufs.cbegin(), bufs.cend(), converted_bufs.begin(), [](const auto vec) {
        return iovec{.iov_base = vec.data(), .iov_len = vec.size_bytes()};
    });

    const auto ret = writev(fd, converted_bufs.data(), bufs.size());
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    assert(ret >= 0);
    return ret;
}

std::expected<size_t, error::ErrorCode> send(
        const int sock, const RefBuffer buf, const int flags) noexcept {
    const auto ret = ::send(sock, buf.as_span().data(), buf.as_span().size_bytes(), flags);
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    return ret;
}

std::expected<size_t, error::ErrorCode> sendmsg(
        const int sock, const ::msghdr& msg, const int flags) noexcept {
    const auto ret = ::sendmsg(sock, &msg, flags);
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    return ret;
}

std::expected<long, error::ErrorCode> sysconf(const int name) noexcept {
    /*
         * From the man pages:
         *
         * If name corresponds to a maximum or minimum limit, and that limit is indeterminate,
         * -1 is returned and errno is not changed.
         * To distinguish an indeterminate limit from an error,
         * set errno to zero before the call, and then check whether errno is
         * nonzero when -1 is returned.
         */
    errno = 0;
    const auto ret = ::sysconf(name);
    if (ret == -1) {
        if (errno == 0) {
            //errno not changing means indeterminate value returned successfully instead of error
            return std::unexpected(error::get_error_code_from_errno(ENOTSUP));
        }
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    return ret;
}

std::expected<size_t, error::ErrorCode> recv(
        const int sock, RefBuffer buf, const int flags) noexcept {
    const auto ret = ::recv(sock, buf.data(), buf.size(), flags);
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    return ret;
}

std::expected<std::pair<size_t, std::variant<n3::net::v4::address, n3::net::v6::address>>,
        error::ErrorCode>
        recvfrom(const int sock, RefBuffer buf, const int flags) noexcept {
    ::sockaddr_storage recv_addr{};
    socklen_t recv_addr_len = sizeof(recv_addr);

    const auto ret = ::recvfrom(sock,
            buf.data(),
            buf.size(),
            flags,
            reinterpret_cast<::sockaddr *>(&recv_addr),
            &recv_addr_len);
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    const auto address = n3::net::sockaddr_to_address(recv_addr);

    return {{ret, address}};
}

std::expected<std::pair<size_t, ::msghdr>, error::ErrorCode> recvmsg(
        const int sock, const int flags) noexcept {
    ::msghdr msg{};
    const auto ret = ::recvmsg(sock, &msg, flags);
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    return {{ret, msg}};
}

std::expected<void, error::ErrorCode> listen(const int sock, const int backlog) noexcept {
    const auto ret = ::listen(sock, backlog);
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    return {};
}

std::expected<std::pair<int, std::variant<n3::net::v4::address, n3::net::v6::address>>,
        error::ErrorCode>
        accept(const int sock) noexcept {
    ::sockaddr_storage recv_addr{};
    socklen_t recv_addr_len = sizeof(recv_addr);

    const auto ret = ::accept(sock, reinterpret_cast<::sockaddr *>(&recv_addr), &recv_addr_len);
    if (ret == -1) {
        return std::unexpected(error::get_error_code_from_errno(errno));
    }
    const auto address = n3::net::sockaddr_to_address(recv_addr);

    return {{ret, address}};
}

} // namespace n3::linux

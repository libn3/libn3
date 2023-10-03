#include <arpa/inet.h>
#include <cstdint>
#include <expected>
#include <fcntl.h>
#include <linux/filter.h>
#include <linux/icmp.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <span>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "socket.h"

#include "error.h"

namespace n3 { namespace net { namespace linux {

    socket::socket() :
            sock(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)) {
        if (this->sock == -1) {
            throw error::get_error_code_from_errno(errno);
        }
    }

    //n3::net::linux::tcp::socket s;
    //s.setsockopt(SO_KEEPALIVE, 1);
    //s.setsockopt<SO_KEEPALIVE>(1);

    //s.getsockopt(SO_KEEPALIVE, 1);
    //s.getsockopt<SO_KEEPALIVE>(1);

    socket::~socket() noexcept {
        //No good way to handle error returns, maybe an eventual "cleanup error callback function?"
        close(this->sock);
    }

    [[nodiscard]] constexpr size_t socket::get_sockopt_size(int level, int optname) const noexcept {
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
                            return sizeof(struct sock_fprog);
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
                            return sizeof(struct linger);
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
                            return sizeof(struct ucred);
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
                            return sizeof(struct timeval);
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
                break;
            case IPPROTO_IPV6:
                break;
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
                            return sizeof(struct tcp_info);
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
                break;
            case IPPROTO_UDP:
                break;
            default:
                std::unreachable();
        }
        return 0;
    }

    //std::expected<void, error::code> socket::setsockopt(const int level,
    //        const int option,
    //        const std::span<std::byte> option_value) const noexcept {
    //    const auto ret = ::setsockopt(
    //            this->sock, level, option, option_value.data(), option_value.size_bytes());
    //    if (ret == -1) {
    //        return std::unexpected(error::get_error_code_from_errno(errno));
    //    }
    //    return {};
    //}

    //std::expected<std::vector<std::byte>, error::code> socket::getsockopt(
    //        const int level, const int option) const noexcept {
    //    const auto ret = ::setsockopt(
    //            this->sock, level, option, option_value.data(), option_value.size_bytes());
    //    if (ret == -1) {
    //        return std::unexpected(error::get_error_code_from_errno(errno));
    //    }
    //    return {};
    //}
}}} // namespace n3::net::linux

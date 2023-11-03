#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "address.h"

namespace n3 { namespace net {
    namespace v4 {
        address::address() noexcept : addr{INADDR_ANY}, port{0} {
        }
        address::address(const ::sockaddr_in& sockaddr) noexcept :
                addr{sockaddr.sin_addr.s_addr},
                port{sockaddr.sin_port} {
        }
    } // namespace v4

    namespace v6 {
        address::address() noexcept :
                addr{std::to_array(in6addr_any.s6_addr)},
                flowinfo{0},
                scope_id{0},
                port{0} {
        }
        address::address(const ::sockaddr_in6& sockaddr) noexcept :
                addr{std::to_array(sockaddr.sin6_addr.s6_addr)},
                flowinfo{0},
                scope_id{0},
                port{sockaddr.sin6_port} {
        }
    } // namespace v6
}}; // namespace n3::net

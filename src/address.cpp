#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "address.h"

#include "dns.h"

namespace n3::net {
namespace v4 {
    address::address() noexcept : addr{INADDR_ANY}, port{0} {
    }
    address::address(const ::sockaddr_in& sockaddr) noexcept :
            addr{sockaddr.sin_addr.s_addr},
            port{sockaddr.sin_port} {
    }
    [[nodiscard]] ::sockaddr_in address::to_sockaddr() const noexcept {
        ::sockaddr_in out;
        out.sin_family = AF_INET;
        out.sin_port = this->port;
        out.sin_addr.s_addr = this->addr;
        return out;
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
    [[nodiscard]] ::sockaddr_in6 address::to_sockaddr() const noexcept {
        ::sockaddr_in6 out;
        out.sin6_family = AF_INET6;
        out.sin6_port = this->port;
        std::copy(this->addr.cbegin(), this->addr.cend(), out.sin6_addr.s6_addr);

        return out;
    }
} // namespace v6
}; // namespace n3::net

#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstring>
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
#include <optional>
#include <span>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "dns.h"

#include "error.h"

namespace n3::net::dns {

addrinfo::addrinfo(const ::addrinfo& caddr) noexcept :
        ai_flags{caddr.ai_flags},
        ai_family{caddr.ai_family},
        ai_socktype{caddr.ai_socktype},
        ai_protocol{caddr.ai_protocol},
        ai_addr{},
        ai_canonname{caddr.ai_canonname} {
    std::memcpy(&this->ai_addr, caddr.ai_addr, caddr.ai_addrlen);
}

[[nodiscard]] const std::expected<std::vector<addrinfo>, n3::error::ErrorCode> getaddrinfo(
        const std::optional<std::string>& node,
        const std::optional<std::string>& service,
        const std::optional<::addrinfo>& hints) {
    if (!node && !service) {
        /*
         * Per glibc documentation:
         * Either node or service, but not both, may be NULL.
         */
        return std::unexpected(error::get_error_code_from_errno(EINVAL));
    }

    const std::unique_ptr<::addrinfo *, void (*)(::addrinfo **)> addr_list{
            nullptr, [](::addrinfo **addr) {
                if (addr) {
                    freeaddrinfo(*addr);
                }
            }};

    int ret = getaddrinfo(node.transform(&std::string::c_str).value_or(nullptr),
            service.transform(&std::string::c_str).value_or(nullptr),
            hints.transform([](const auto& hints_arg) { return &hints_arg; }).value_or(nullptr),
            addr_list.get());
    if (ret != 0) {
        return std::unexpected(error::get_error_code_from_getaddrinfo_err(ret, errno));
    }
    assert(addr_list);

    std::vector<addrinfo> result_vec;

    //Travel the linked list of results and add them to the vector
    for (::addrinfo *idx = *addr_list; idx != nullptr; idx = idx->ai_next) {
        result_vec.emplace_back(*idx);
    }

    return result_vec;
}

}; // namespace n3::net::dns

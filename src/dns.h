#pragma once

#include <expected>
#include <optional>
#include <string>
#include <sys/types.h>
#include <vector>

#include "error.h"

namespace n3::net::dns {

/*
 * TODO: Can probably avoid copies of data in this class if I make it own a refcount that
 * controls when we call freeaddrinfo()
 */
class addrinfo {
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    //TODO: Worth having my own wrapper of a variant over the sockaddr types?
    struct sockaddr_storage ai_addr;
    std::string ai_canonname;

public:
    addrinfo(const ::addrinfo& caddr) noexcept;
};

//TODO: This can throw from dynamic memory allocation, what is my strategy for that?
[[nodiscard]] const std::expected<std::vector<addrinfo>, n3::error::code> getaddrinfo(
        const std::optional<std::string>& node,
        const std::optional<std::string>& service,
        const std::optional<::addrinfo>& hints = std::nullopt);

}; // namespace n3::net::dns

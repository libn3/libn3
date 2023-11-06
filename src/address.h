#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <type_traits>

namespace n3::net {

//Concept to constrain C style sockaddr types to our custom wrapper types
template<typename T>
concept AddressType = requires(T addr) {
    { addr.to_sockaddr() };
};

namespace v4 {

    class address {
        uint32_t addr;
        uint16_t port;

    public:
        address() noexcept;
        address(const ::sockaddr_in& sockaddr) noexcept;
        [[nodiscard]] ::sockaddr_in to_sockaddr() const noexcept;
    };

} // namespace v4

namespace v6 {

    class address {
        std::array<uint8_t, 16> addr;
        uint32_t flowinfo;
        uint32_t scope_id;
        uint16_t port;

    public:
        address() noexcept;
        address(const ::sockaddr_in6& sockaddr) noexcept;
        [[nodiscard]] ::sockaddr_in6 to_sockaddr() const noexcept;
    };
} // namespace v6
}; // namespace n3::net

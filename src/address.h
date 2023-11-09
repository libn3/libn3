#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <type_traits>
#include <utility>
#include <variant>

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
        [[nodiscard]] constexpr ::sockaddr_in to_sockaddr() const noexcept;
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
        [[nodiscard]] constexpr ::sockaddr_in6 to_sockaddr() const noexcept;
    };
} // namespace v6

//TODO: Is templating the return type with the AddressType concept a better design?
[[nodiscard]] constexpr auto sockaddr_to_address(const ::sockaddr_storage& addr) noexcept
        -> std::variant<v4::address, v6::address> {
    switch (addr.ss_family) {
        case AF_INET:
            return {reinterpret_cast<const ::sockaddr_in&>(addr)};
        case AF_INET6:
            return {reinterpret_cast<const ::sockaddr_in6&>(addr)};
        default:
            //TODO: Could be sockaddr_un or have family AF_UNSPEC, need to support those eventually
            std::unreachable();
            break;
    }
}

}; // namespace n3::net

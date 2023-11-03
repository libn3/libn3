#pragma once

#include <array>
#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

namespace n3 { namespace net {
    namespace v4 {

        class address {
            uint32_t addr;
            uint16_t port;

        public:
            address() noexcept;
            address(const ::sockaddr_in& sockaddr) noexcept;
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
        };
    } // namespace v6
}}; // namespace n3::net

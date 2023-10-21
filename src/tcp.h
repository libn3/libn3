#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include "error.h"
#include "socket.h"

namespace n3 { namespace net { namespace linux { namespace tcp {

    class socket : public n3::net::linux::socket {
        const int sock;

    public:
        socket();
        socket(const int sock);
        socket(const std::string_view ip_str, const std::string_view port_str);
        socket(const std::string_view ip_str, const uint16_t port_str);

        ~socket() noexcept;

        socket(const socket&) = delete;
        socket(socket&&) noexcept = default;

        socket& operator=(const socket&) = delete;
        socket& operator=(socket&&) = default;
    };

}}}} // namespace n3::net::linux::tcp

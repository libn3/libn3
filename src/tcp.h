#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <utility>
#include <vector>
#include <string_view>

#include "error.h"
#include "socket.h"

namespace n3 { namespace net { namespace linux { namespace tcp {

    class socket : public n3::net::linux::socket<int> {
    public:
        socket() = default;
        socket(const std::string_view ip_str, const std::string_view port_str);
        socket(const std::string_view ip_str, const uint16_t port_str);
    };

}}}} // namespace n3::net::linux::tcp

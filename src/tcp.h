#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include "error.h"
#include "socket.h"

namespace n3::net::linux::tcp {

class TcpSocket : public n3::net::linux::socket<TcpSocket> {
    const int sock;

public:
    TcpSocket();
    TcpSocket(const int sock_arg);
    TcpSocket(const std::string_view ip_str, const std::string_view port_str);
    TcpSocket(const std::string_view ip_str, const uint16_t port_str);

    ~TcpSocket() noexcept;

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket(TcpSocket&&) noexcept = default;

    TcpSocket& operator=(const TcpSocket&) = delete;
    TcpSocket& operator=(TcpSocket&&) = default;
};

} // namespace n3::net::linux::tcp

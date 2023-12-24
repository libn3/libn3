#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <climits>
#include <cstdint>
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
#include <span>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "tcp.h"

#include "error.h"
#include "handle.h"

namespace n3::net::linux::tcp {

TcpSocket::TcpSocket() :
        sock(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)) {
    if (this->sock == -1) {
        throw error::get_error_code_from_errno(errno);
    }
}

TcpSocket::~TcpSocket() noexcept {
    //Shutdown the TCP socket cleanly
    shutdown(this->sock, SHUT_RDWR);
    //Socket fd close handled by the OwnedHandle destructor
}

TcpSocket::TcpSocket(const Handle sock_arg) : sock{sock_arg} {
}

//socket::socket(const std::string_view ip_str, const std::string_view port_str) {
//    sock = 0;
//}
//socket::socket(const std::string_view ip_str, const uint16_t port_str) {
//    sock = 0;
//}

} // namespace n3::net::linux::tcp

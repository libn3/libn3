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

namespace n3 { namespace net { namespace linux { namespace tcp {

    socket::socket(const std::string_view ip_str, const std::string_view port_str) {
        //test
    }
    socket::socket(const std::string_view ip_str, const uint16_t port_str) {
        //test
    }

}}}} // namespace n3::net::linux::tcp

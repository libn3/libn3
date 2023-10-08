#include <arpa/inet.h>
#include <cstdint>
#include <expected>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <span>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
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

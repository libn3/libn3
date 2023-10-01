#include <arpa/inet.h>
#include <expected>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <span>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "tcp.h"

#include "error.h"

namespace n3 { namespace tcp {

    socket::socket() :
            sock(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)) {
        if (this->sock == -1) {
            throw error::get_error_code_from_errno(errno);
        }
    }

    socket::~socket() noexcept {
        //No good way to handle error returns, maybe an eventual "cleanup error callback function?"
        close(this->sock);
    }

    //std::expected<void, error::code> socket::setsockopt(const int level,
    //        const int option,
    //        const std::span<std::byte> option_value) const noexcept {
    //    const auto ret = ::setsockopt(
    //            this->sock, level, option, option_value.data(), option_value.size_bytes());
    //    if (ret == -1) {
    //        return std::unexpected(error::get_error_code_from_errno(errno));
    //    }
    //    return {};
    //}

    //std::expected<std::vector<std::byte>, error::code> socket::getsockopt(
    //        const int level, const int option) const noexcept {
    //    const auto ret = ::setsockopt(
    //            this->sock, level, option, option_value.data(), option_value.size_bytes());
    //    if (ret == -1) {
    //        return std::unexpected(error::get_error_code_from_errno(errno));
    //    }
    //    return {};
    //}
}} // namespace n3::tcp

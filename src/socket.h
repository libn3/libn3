#pragma once

#include <expected>
#include <span>
#include <utility>
#include <vector>

#include "error.h"

namespace n3 { namespace net { namespace linux {

    class socket {

        [[nodiscard]] constexpr size_t get_sockopt_size(int level, int optname) const noexcept;

    public:
        const int sock;

        socket();
        ~socket() noexcept;

        socket(const socket&) = delete;
        socket(socket&&) noexcept = default;

        socket& operator=(const socket&) = delete;
        socket& operator=(socket&&) = default;

        std::expected<void, error::code> setsockopt(const int level,
                const int option,
                const std::span<std::byte> option_value) const noexcept;

        std::expected<std::vector<std::byte>, error::code> getsockopt(
                const int level, const int option) const noexcept;
    };

}}} // namespace n3::net::linux
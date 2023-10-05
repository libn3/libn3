#pragma once

#include <expected>
#include <span>
#include <utility>
#include <vector>

#include "error.h"

namespace n3 { namespace net { namespace linux {

    class socket {
        std::vector<std::byte> write_buffer;
        std::vector<std::byte> read_buffer;

        [[nodiscard]] static constexpr size_t get_sockopt_size(int level, int optname) noexcept;
        std::expected<size_t, error::code> write_vectored(
                const std::span<const std::span<std::byte>> bufs) const noexcept;

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

        std::expected<std::span<std::byte>, error::code> getsockopt(const int level,
                const int option,
                const std::span<std::byte> option_buf) const noexcept;

        std::expected<size_t, error::code> send(const std::span<std::byte> buf) const noexcept;
        std::expected<size_t, error::code> send(
                const std::span<std::span<std::byte>> bufs) const noexcept;
    };

}}} // namespace n3::net::linux

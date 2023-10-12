#pragma once

#include <expected>
#include <span>

#include "error.h"

namespace n3 { namespace linux {
    //TODO: Probably need to enum and error check all the different int args to prevent misuse

    [[nodiscard]] constexpr size_t get_sockopt_size(int level, int optname) noexcept;

    std::expected<void, error::code> setsockopt(const int sock, const int level,
            const int option,
            const std::span<std::byte> option_value) noexcept;

    std::expected<std::span<std::byte>, error::code> getsockopt(const int sock, const int level,
            const int option,
            const std::span<std::byte> option_buf) noexcept;

    //std::expected<size_t, error::code> send(const std::span<std::byte> buf) noexcept;
}} // namespace n3::linux

#pragma once

#include <concepts>
#include <memory>
#include <span>
#include <sys/uio.h>
#include <type_traits>
#include <utility>
#include <vector>

namespace ns {

template<typename T, typename... Args>
concept NoThrowConstructible
        = std::constructible_from<T, Args...> && std::is_nothrow_constructible_v<T, Args...>;

class OwningBuffer {
    std::vector<std::byte> data;

public:
    OwningBuffer();
    OwningBuffer(std::span<std::byte> init_data);
    OwningBuffer(std::vector<std::byte>&& init_data);
};

class RefBuffer {
    std::span<std::byte> data;

public:
    RefBuffer() = default;

    //Constructor for anything that can normally make an std::span<std::byte>
    template<typename... Args>
        requires NoThrowConstructible<decltype(data), Args...>
    RefBuffer(Args&&...args) noexcept : data{std::forward<decltype(args)>(args)...} {
    }

    //Constructor for other types of spans that can be converted to a span of bytes
    template<typename T>
    RefBuffer(std::span<T>&& sp) noexcept :
            data{std::forward<decltype(sp)>(std::as_writable_bytes(sp))} {
    }

    //Constructor for iovecs which are basically just a span of bytes
    RefBuffer(const struct iovec iov) noexcept :
            data{static_cast<std::byte *>(iov.iov_base), iov.iov_len} {
    }

    [[nodiscard]] constexpr auto as_iovec() noexcept -> struct iovec {
        return {
                .iov_base = this->data.data(),
                .iov_len = this->data.size_bytes(),
        };
    }
};

//Compiler errors to make sure a RefBuffer is as trivial as it should be
static_assert(std::is_trivially_copyable_v<RefBuffer>);
static_assert(std::is_trivially_copy_constructible_v<RefBuffer>);
static_assert(std::is_trivially_move_constructible_v<RefBuffer>);
static_assert(std::is_trivially_copy_assignable_v<RefBuffer>);
static_assert(std::is_trivially_move_assignable_v<RefBuffer>);
static_assert(std::is_trivially_destructible_v<RefBuffer>);

class RefMultiBuffer {
    std::span<RefBuffer> buffers;

public:
    RefMultiBuffer() = default;

    //Constructor for anything that can normally make an std::span<RefBuffer>
    template<typename... Args>
        requires NoThrowConstructible<decltype(buffers), Args...>
    RefMultiBuffer(Args&&...args) noexcept : buffers{std::forward<decltype(args)>(args)...} {
    }

    //TODO: How to do this properly?
    //[[nodiscard]] constexpr auto as_iovec() noexcept -> struct iovec {
    //    return {
    //            .iov_base = this->buffers.buffers(),
    //            .iov_len = this->buffers.size_bytes(),
    //    };
    //}
};

//Compiler errors to make sure a RefMultiBuffer is as trivial as it should be
static_assert(std::is_trivially_copyable_v<RefMultiBuffer>);
static_assert(std::is_trivially_copy_constructible_v<RefMultiBuffer>);
static_assert(std::is_trivially_move_constructible_v<RefMultiBuffer>);
static_assert(std::is_trivially_copy_assignable_v<RefMultiBuffer>);
static_assert(std::is_trivially_move_assignable_v<RefMultiBuffer>);
static_assert(std::is_trivially_destructible_v<RefMultiBuffer>);

} // namespace ns

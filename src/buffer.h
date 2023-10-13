#pragma once

#include <cassert>
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
    RefBuffer() noexcept = default;

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

    //Constructor for arbitrary type pointers which can always be aliased by std::byte
    template<typename T>
    RefBuffer(T *const buf, const size_t len) noexcept :
            data{reinterpret_cast<std::byte *>(buf), len} {
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

    constexpr std::byte& operator[](size_t idx) const noexcept {
        assert(idx <= data.size_bytes());
        return data[idx];
    }
};

//Compiler errors to make sure a RefBuffer is as trivial as it should be
static_assert(std::is_trivially_copyable_v<RefBuffer>);
static_assert(std::is_trivially_copy_constructible_v<RefBuffer>);
static_assert(std::is_trivially_move_constructible_v<RefBuffer>);
static_assert(std::is_trivially_copy_assignable_v<RefBuffer>);
static_assert(std::is_trivially_move_assignable_v<RefBuffer>);
static_assert(std::is_trivially_destructible_v<RefBuffer>);
static_assert(std::is_nothrow_copy_constructible_v<RefBuffer>);
static_assert(std::is_nothrow_move_constructible_v<RefBuffer>);
static_assert(std::is_nothrow_copy_assignable_v<RefBuffer>);
static_assert(std::is_nothrow_move_assignable_v<RefBuffer>);

template<size_t size>
class RefMultiBuffer {
    std::array<RefBuffer, size> buffers;
    //Don't bother with making a multibuffer larger than a single syscall can take
    static_assert(size <= IOV_MAX);

public:
    RefMultiBuffer() noexcept = default;

    //Constructor for anything that can normally make an std::span<RefBuffer>
    template<typename... Args>
        requires NoThrowConstructible<decltype(buffers), Args...>
    RefMultiBuffer(Args&&...args) noexcept : buffers{std::forward<decltype(args)>(args)...} {
    }

    constexpr RefBuffer& operator[](size_t idx) const noexcept {
        assert(idx <= size);
        return buffers[idx];
    }

    //TODO: How to do this properly?
    //[[nodiscard]] constexpr auto as_iovec() noexcept -> struct iovec {
    //    return {
    //            .iov_base = this->buffers.buffers(),
    //            .iov_len = this->buffers.size_bytes(),
    //    };
    //}
};

static_assert(std::is_trivially_copyable_v<RefMultiBuffer<IOV_MAX>>);
static_assert(std::is_nothrow_copy_constructible_v<RefMultiBuffer<IOV_MAX>>);
static_assert(std::is_nothrow_move_constructible_v<RefMultiBuffer<IOV_MAX>>);
static_assert(std::is_nothrow_copy_assignable_v<RefMultiBuffer<IOV_MAX>>);
static_assert(std::is_nothrow_move_assignable_v<RefMultiBuffer<IOV_MAX>>);

//TODO: This may be useful as an optimization, std::span equivalent with std::byte* didn't work
static_assert(std::is_layout_compatible_v<struct iovec, std::pair<void *, size_t>>);

} // namespace ns

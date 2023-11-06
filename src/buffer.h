#pragma once

#include <cassert>
#include <concepts>
#include <memory>
#include <ranges>
#include <span>
#include <sys/uio.h>
#include <type_traits>
#include <utility>
#include <vector>

namespace n3 {

/*
 * TODO: ConstRefBuffer et al to explicitly have std::span<const std::byte>?
 */

/*
 * TODO: Naming?
 * We use both camel_case and PascalCase for our names, what is the convention I'm using?
 */

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

class RefBuffer : public std::ranges::view_interface<RefBuffer> {
    std::span<std::byte> underlying;

public:
    RefBuffer() noexcept = default;

    //Constructor for anything that can normally make an std::span<std::byte>
    template<typename... Args>
        requires NoThrowConstructible<decltype(underlying), Args...>
    RefBuffer(Args&&...args) noexcept : underlying{std::forward<decltype(args)>(args)...} {
    }

    //Constructor for other types of spans that can be converted to a span of bytes
    template<typename T>
    RefBuffer(std::span<T>&& sp) noexcept :
            underlying{std::forward<decltype(sp)>(std::as_writable_bytes(sp))} {
    }

    //Constructor for arbitrary type pointers which can always be aliased by std::byte
    template<typename T>
    RefBuffer(T *const buf, const size_t len) noexcept :
            underlying{reinterpret_cast<std::byte *>(buf), len} {
    }

    //Constructor for iovecs which are basically just a span of bytes
    RefBuffer(const struct iovec iov) noexcept :
            underlying{static_cast<std::byte *>(iov.iov_base), iov.iov_len} {
    }

    [[nodiscard]] constexpr auto data() const noexcept -> std::byte * {
        return this->underlying.data();
    }

    [[nodiscard]] constexpr auto size() const noexcept -> size_t {
        return this->underlying.size_bytes();
    }

    [[nodiscard]] constexpr auto as_iovec() const noexcept -> struct iovec {
        return {
                .iov_base = this->underlying.data(),
                .iov_len = this->underlying.size_bytes(),
        };
    }

    [[nodiscard]] constexpr auto
            as_span() const noexcept -> std::span<std::byte> {
        return underlying;
    }

    constexpr std::byte& operator[](size_t idx) const noexcept {
        assert(idx <= underlying.size_bytes());
        return underlying[idx];
    }

    [[nodiscard]] constexpr auto begin() const noexcept -> std::byte * {
        return this->underlying.data();
    }
    [[nodiscard]] constexpr auto end() const noexcept -> std::byte * {
        return this->underlying.data() + this->underlying.size_bytes();
    }
    [[nodiscard]] constexpr auto cbegin() const noexcept -> const std::byte * {
        return this->underlying.data();
    }
    [[nodiscard]] constexpr auto cend() const noexcept -> const std::byte * {
        return this->underlying.data() + this->underlying.size_bytes();
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

/*
 * A simple memory page buffer that checks the page size at runtime
 */
class PageBuffer {
    const size_t page_size;
    const std::unique_ptr<std::byte[]> underlying;

public:
    PageBuffer();

    //TODO: Add operator* and operator-> as aliases for returning a span for usability?

    const std::span<std::byte> data() const noexcept {
        return std::span{this->underlying.get(), this->page_size};
    }
};

} // namespace n3

//Specialization in std to mark RefBuffer as a borrowed range since it's basically just a span
template<>
inline constexpr bool std::ranges::enable_borrowed_range<n3::RefBuffer> = true;

//Make sure the refbuffer is a trivial range type, asserts need to be after the namespace std specialization
static_assert(std::ranges::borrowed_range<n3::RefBuffer>);
static_assert(std::ranges::contiguous_range<n3::RefBuffer>);

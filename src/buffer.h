#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <deque>
#include <functional>
#include <memory>
#include <ranges>
#include <span>
#include <sys/uio.h>
#include <type_traits>
#include <utility>
#include <vector>

#include "callbacks.h"

namespace n3 {

/*
 * TODO: ConstRefBuffer et al to explicitly have std::span<const std::byte>?
 */

/*
 * TODO: Naming?
 * We use both camel_case and PascalCase for our names, what is the convention I'm using?
 */

class OwningBuffer {
    std::vector<std::byte> data;

public:
    OwningBuffer();
    OwningBuffer(std::span<std::byte> init_data);
    OwningBuffer(std::vector<std::byte>&& init_data);
};

class RefBuffer : public std::ranges::view_interface<RefBuffer> {
    /*
     * Using an iovec as the underlying instead of a span of bytes because they aren't layout
     * compatible types, and otherwise I would need to do conversion to use them in a
     * scatter-gather context like RefMultiBuffer
     *
     * Much of the implementation code relies on the fact std::byte is a special aliasing type,
     * much like char/unsigned char, so we can get away with static casts for everything
     */
    ::iovec underlying;

    /*
     * This function needs to exist to play nicely with the static asserts it contains
     * Specifically, std::is_pointer_interconvertible_with_class for the underlying iovec
     * Declare it inside the top level of the class, class type is incomplete
     * Declare it outside the class, member is private
     * The rest of the asserts were added here just to keep them all together
     */
    consteval void assert_guarantees() const noexcept {
        //Compiler errors to make sure a RefBuffer is as trivial as it should be
        static_assert(std::is_trivial_v<RefBuffer>);
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

        //Some sanity asserts about the memory layout
        static_assert(std::is_nothrow_convertible_v<RefBuffer, ::iovec>);
        static_assert(std::is_standard_layout_v<RefBuffer>);
        static_assert(std::is_standard_layout_v<::iovec>);
        static_assert(sizeof(RefBuffer) == sizeof(::iovec));
        static_assert(alignof(RefBuffer) == alignof(::iovec));

        //Assert that we can reinterpret_cast<::iovec *>(RefBuffer *) without UB
        static_assert(std::is_pointer_interconvertible_with_class(&RefBuffer::underlying));
    }

public:
    //Default constructor would mean nullptr which we don't want to allow
    constexpr RefBuffer() = delete;

    //Constructor for anything that can normally make an ::iovec
    constexpr RefBuffer(auto&&...args) noexcept(
            std::is_nothrow_constructible_v<decltype(underlying), decltype(args)...>) :
            underlying{std::forward<decltype(args)>(args)...} {
        //Assert that all the types in the parameter pack aren't the null pointer
        static_assert((std::is_null_pointer_v<decltype(args)> && ...));
    }

    //Constructor for other types of spans that can be converted to a span of bytes
    template<typename T>
        requires std::is_trivially_copyable_v<T>
    constexpr RefBuffer(std::span<T> sp) noexcept :
            underlying{static_cast<void *>(sp.data()), sp.size_bytes()} {
    }

    //Constructor for arbitrary type pointers which can always be cast to void* in an iovec
    template<typename T>
        requires(std::is_trivially_copyable_v<T> + !std::is_null_pointer_v<T>)
    constexpr RefBuffer(T *const buf) noexcept : underlying{static_cast<void *>(buf), sizeof(T)} {
    }

    [[nodiscard]] constexpr auto data() const noexcept -> std::byte * {
        return reinterpret_cast<std::byte *>(this->underlying.iov_base);
    }

    [[nodiscard]] constexpr auto size() const noexcept -> size_t {
        return this->underlying.iov_len;
    }

    [[nodiscard]] constexpr auto as_iovec() const noexcept -> ::iovec {
        return this->underlying;
    }

    [[nodiscard]] constexpr auto as_span() const noexcept -> std::span<std::byte> {
        return {reinterpret_cast<std::byte *>(this->underlying.iov_base), this->underlying.iov_len};
    }

    [[nodiscard]] constexpr operator ::iovec() const noexcept {
        return this->underlying;
    }
    [[nodiscard]] constexpr operator std::span<std::byte>() const noexcept {
        return this->as_span();
    }

    [[nodiscard]] constexpr std::byte& operator[](size_t idx) const noexcept {
        assert(idx <= this->underlying.iov_len);
        return reinterpret_cast<std::byte *>(this->underlying.iov_base)[idx];
    }

    [[nodiscard]] constexpr auto begin() const noexcept -> std::byte * {
        return this->data();
    }
    [[nodiscard]] constexpr auto end() const noexcept -> std::byte * {
        return this->data() + this->size();
    }
    [[nodiscard]] constexpr auto cbegin() const noexcept -> const std::byte * {
        return this->data();
    }
    [[nodiscard]] constexpr auto cend() const noexcept -> const std::byte * {
        return this->data() + this->size();
    }
};

class RefMultiBuffer {
    std::vector<RefBuffer> buffers;

public:
    //Default constructor
    constexpr RefMultiBuffer() = default;

    //Move constructible only
    constexpr RefMultiBuffer(const RefMultiBuffer&) = delete;
    constexpr RefMultiBuffer(RefMultiBuffer&&) = default;

    //Constructor for anything that can normally make an std::vector<RefBuffer>
    constexpr RefMultiBuffer(auto&&...args) noexcept(
            std::is_nothrow_constructible_v<decltype(this->buffers), decltype(args)...>) :
            buffers{std::forward<decltype(args)>(args)...} {
    }

    //Move assignable only
    constexpr RefMultiBuffer& operator=(const RefMultiBuffer&) = delete;
    constexpr RefMultiBuffer& operator=(RefMultiBuffer&&) = default;

    [[nodiscard]] constexpr auto data() noexcept -> RefBuffer * {
        return this->buffers.data();
    }
    [[nodiscard]] constexpr auto data() const noexcept -> const RefBuffer * {
        return this->buffers.data();
    }

    [[nodiscard]] constexpr auto size() const noexcept -> size_t {
        return this->buffers.size();
    }
    [[nodiscard]] constexpr auto size_bytes() const noexcept -> size_t {
        return std::ranges::fold_left(
                this->buffers | std::views::transform(&RefBuffer::size), 0, std::plus<size_t>());
    }

    [[nodiscard]] constexpr auto begin() noexcept -> RefBuffer * {
        return this->buffers.data();
    }
    [[nodiscard]] constexpr auto begin() const noexcept -> const RefBuffer * {
        return this->buffers.data();
    }
    [[nodiscard]] constexpr auto cbegin() const noexcept -> const RefBuffer * {
        return this->buffers.data();
    }
    [[nodiscard]] constexpr auto end() noexcept -> RefBuffer * {
        return this->buffers.data() + this->buffers.size();
    }
    [[nodiscard]] constexpr auto end() const noexcept -> const RefBuffer * {
        return this->buffers.data() + this->buffers.size();
    }
    [[nodiscard]] constexpr auto cend() const noexcept -> const RefBuffer * {
        return this->buffers.data() + this->buffers.size();
    }

    /*
     * TODO: Implement constructor for pointers and non-const lvalue-references
     * Would be useful to transparently switch to scatter-gather IO based on usage
     * Requires some fancy concept constraints to make sure we can make iovecs as expected
     * For example:
     *
     * TcpSocket s{};
     * s.connect("127.0.0.1", 80);
     *
     * int x = 0;
     * uint32_t y = 0xdeadbeef;
     * std::string body = "Some longer string of text";
     *
     * s.send(x, y, body); //This would call writev et al under the hood
     */

    [[nodiscard]] constexpr RefBuffer& operator[](const size_t idx) noexcept {
        assert(idx <= buffers.size());
        return buffers[idx];
    }
    [[nodiscard]] constexpr const RefBuffer& operator[](const size_t idx) const noexcept {
        assert(idx <= buffers.size());
        return buffers[idx];
    }

    [[nodiscard]] constexpr operator ::iovec *() noexcept {
        assert(!this->buffers.empty());
        return reinterpret_cast<::iovec *>(this->buffers.data());
    }
    [[nodiscard]] constexpr operator const ::iovec *() const noexcept {
        assert(!this->buffers.empty());
        return reinterpret_cast<const ::iovec *>(this->buffers.data());
    }

    constexpr void push_back(const RefBuffer& buf) {
        this->buffers.push_back(buf);
    }
    constexpr void push_back(RefBuffer&& buf) {
        this->buffers.push_back(std::move(buf));
    }

    constexpr auto emplace_back(auto&&...args) {
        return this->buffers.emplace_back(std::forward<decltype(args)>(args)...);
    }

    constexpr void extend(const RefMultiBuffer& multi) {
        std::ranges::copy(multi.buffers, std::back_inserter(this->buffers));
    }
    constexpr void extend(RefMultiBuffer&& multi) {
        std::ranges::move(multi.buffers, std::back_inserter(this->buffers));
    }

    constexpr void consume(const size_t bytes) {
        assert(bytes <= this->size_bytes());

        //Remove buffers from the head while the total size is smaller than the requested amount
        size_t sum = 0;
        std::erase_if(this->buffers, [&](const auto& buf) {
            const auto remaining = (bytes - sum);
            const bool should_erase = (buf.size() <= remaining);
            sum += (should_erase) ? buf.size() : remaining;
            return should_erase;
        });
        assert(sum <= bytes);

        //Should only happen when bytes is equal to the total number of bytes stored in the buffers
        if (this->buffers.empty()) {
            assert(sum == bytes);
            return;
        }

        const auto remaining = (bytes - sum);
        assert(remaining > 0);

        auto& head_buffer = this->buffers.front();
        //Initial buffer size <= remaining should have been handled by the std::erase_if condition
        assert(head_buffer.size() > remaining);

        //Partial consumption of the remaining head buffer
        head_buffer = head_buffer.as_span() | std::views::drop(remaining);
    }
};

static_assert(std::is_nothrow_default_constructible_v<RefMultiBuffer>);
static_assert(std::is_nothrow_move_constructible_v<RefMultiBuffer>);
static_assert(std::is_nothrow_move_assignable_v<RefMultiBuffer>);

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

class BufferQueue {
    RefMultiBuffer buffers;
    std::deque<std::pair<n3::callback<void>, size_t>> callbacks;
    size_t buffer_size;
    size_t buffer_bytes_size;

public:
    //Default constructor
    BufferQueue() = default;

    //Move constructible only
    BufferQueue(const BufferQueue&) = delete;
    BufferQueue(BufferQueue&&) = default;

    //Move assignable only
    BufferQueue& operator=(const BufferQueue&) = delete;
    BufferQueue& operator=(BufferQueue&&) = default;

    [[nodiscard]] constexpr auto empty() const noexcept -> bool {
        return this->buffer_size == 0;
    }
    [[nodiscard]] constexpr auto size() const noexcept -> size_t {
        return this->buffer_size;
    }

    constexpr void push(const RefBuffer buf, n3::callback<void>&& callback) {
        const auto arg_buf_size_bytes = buf.size();

        this->buffer_size++;
        this->buffer_bytes_size += arg_buf_size_bytes;
        this->buffers.push_back(std::move(buf));
        this->callbacks.emplace_back(std::move(callback), arg_buf_size_bytes);
    }
    constexpr void push(const RefMultiBuffer multi, n3::callback<void>&& callback) {
        const auto arg_buf_size = multi.size();
        const auto arg_buf_size_bytes = multi.size_bytes();

        this->buffer_size += arg_buf_size;
        this->buffer_bytes_size += arg_buf_size_bytes;
        this->buffers.extend(std::move(multi));
        this->callbacks.emplace_back(std::move(callback), arg_buf_size_bytes);
    }
    constexpr void pop(const size_t bytes) {
        if (this->empty()) {
            return;
        }

        assert(bytes <= buffer_bytes_size);
        assert(!this->callbacks.empty());

        this->buffers.consume(bytes);
        this->buffer_size = this->buffers.size();

        size_t remaining = bytes;
        while (remaining > 0) {
            assert(!this->callbacks.empty());
            auto& [cb, cb_size] = this->callbacks.front();
            if (cb_size <= remaining) {
                remaining -= cb_size;
                std::invoke(std::move(cb));
                this->callbacks.pop_front();
                /*
                 * pop_front() invalidates the .front() references above, so explicitly
                 * continue here to prevent any accidental further loop logic using them
                 */
                continue;
            }
            assert(cb_size > remaining);
            cb_size -= remaining;
            remaining = 0;
            break;
        }
    }
};

} // namespace n3

//Specialization to mark RefBuffer as a borrowed range since it's basically just a span
template<>
inline constexpr bool std::ranges::enable_borrowed_range<n3::RefBuffer> = true;

//Make sure the refbuffer is a trivial range type, asserts need to be after the std specialization
static_assert(std::ranges::borrowed_range<n3::RefBuffer>);
static_assert(std::ranges::contiguous_range<n3::RefBuffer>);

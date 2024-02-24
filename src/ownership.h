#pragma once

#include <concepts>
#include <coroutine>
#include <optional>
#include <utility>

namespace n3 {

template<typename T>
    requires std::movable<std::decay_t<T>>
class MoveOnly {
    std::optional<std::decay_t<T>> inner;

public:
    constexpr MoveOnly(void) = default;

    template<typename... Args>
        requires std::constructible_from<std::decay_t<T>, Args...>
    constexpr MoveOnly(Args&&...args) :
            inner{std::make_optional<std::decay_t<T>>(std::forward<decltype(args)>(args)...)} {
    }

    //Move constructible only
    constexpr MoveOnly(const MoveOnly&) = delete;
    constexpr MoveOnly(MoveOnly&& other) noexcept(noexcept(std::exchange(other.inner, std::nullopt))
            && std::is_nothrow_move_constructible_v<decltype(inner)>) :
            inner{std::move(std::exchange(other.inner, std::nullopt))} {
    }

    //Move assignable only
    constexpr MoveOnly& operator=(const MoveOnly&) = delete;
    constexpr MoveOnly& operator=(MoveOnly&& other) noexcept(
            noexcept(std::exchange(other.inner, std::nullopt))
            && std::is_nothrow_move_assignable_v<decltype(inner)>) {
        this->inner = std::move(std::exchange(other.inner, std::nullopt));
        return *this;
    }

    [[nodiscard]] constexpr operator bool() const noexcept {
        return this->inner.has_value();
    }
    [[nodiscard]] constexpr bool has_value() const noexcept {
        return this->inner.has_value();
    }

    /*
     * Need the deleted conversion operators to be explicit to resolve overload resolution in
     * the case that T is implicitly convertable to bool such as "int", otherwise it conflicts
     * with the bool conversion we define
     */
    constexpr explicit operator std::optional<std::decay_t<T>>() = delete;
    constexpr explicit operator std::decay_t<T>() = delete;

    constexpr T *operator->() noexcept {
        return &*this->inner;
    }
    constexpr const T *operator->() const noexcept {
        return &*this->inner;
    }

    constexpr T& operator*() & noexcept {
        return *this->inner;
    }
    constexpr const T& operator*() const& noexcept {
        return *this->inner;
    }
    constexpr const T& operator*() const&& noexcept {
        return *this->inner;
    }
    constexpr T&& operator*() && noexcept {
        return std::move(*this->inner);
    }
};

template<typename T = void>
class OwnedCoroutine {
    using HandleType = std::coroutine_handle<T>;

    MoveOnly<HandleType> coro;

public:
    OwnedCoroutine() : coro{nullptr} {
    }
    OwnedCoroutine(HandleType&& handle) noexcept : coro{std::move(handle)} {
    }

    OwnedCoroutine(auto&&...args) noexcept(
            std::is_nothrow_constructible_v<decltype(this->coro), decltype(args)...>) :
            coro{std::forward<decltype(args)>(args)...} {
    }

    ~OwnedCoroutine() {
        if (this->coro.has_value()) {
            this->coro->destroy();
        }
    }

    constexpr operator OwnedCoroutine<>() const noexcept {
        return auto{std::coroutine_handle<>::from_address(this->coro.address())};
    }

    [[nodiscard]] constexpr std::strong_ordering operator<=>(const OwnedCoroutine&) const noexcept
            = default;
    [[nodiscard]] constexpr std::strong_ordering operator<=>(
            const std::coroutine_handle<>& other) const noexcept {
        return this->coro <=> other;
    }

    [[nodiscard]] bool done() const {
        return this->coro->done();
    }

    void resume() const {
        return this->coro->resume();
    }

    [[nodiscard]] auto& promise() const {
        return this->coro->promise();
    }
};

}; // namespace n3

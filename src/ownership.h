#pragma once

#include <concepts>
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

}; // namespace n3

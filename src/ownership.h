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
    constexpr MoveOnly(void)
        requires(std::is_default_constructible_v<std::decay_t<T>>)
            : inner{} {
    }
    template<typename... Args>
        requires std::constructible_from<std::decay_t<T>, Args...>
    constexpr MoveOnly(Args&&...args) :
            inner{std::make_optional(std::forward<decltype(args)>(args)...)} {
    }

    //Move constructible only
    constexpr MoveOnly(const MoveOnly&) = delete;
    constexpr MoveOnly(MoveOnly&& other) : inner{std::exchange(other.inner, std::nullopt)} {
    }

    //Move assignable only
    constexpr MoveOnly& operator=(const MoveOnly&) = delete;
    constexpr MoveOnly& operator=(MoveOnly&& other) {
        this->inner = std::exchange(other.inner, std::nullopt);
        return *this;
    }

    [[nodiscard]] constexpr operator bool() noexcept {
        return this->inner.has_value();
    }

    [[nodiscard]] constexpr operator std::optional<std::decay_t<T>>() noexcept {
        return std::exchange(this->inner, std::nullopt);
    }

    constexpr T *operator->() noexcept {
        return &*this->inner;
    }
    constexpr const T *operator->() const noexcept {
        return &*this->inner;
    }

    constexpr T& operator*() noexcept {
        return *this->inner;
    }
    constexpr const T& operator*() const noexcept {
        return *this->inner;
    }
};

static constexpr MoveOnly<int> test{};
static constexpr MoveOnly<int> mytest{3};

}; // namespace n3

#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <ranges>
#include <vector>

template<typename T>
class Heap {
    std::vector<std::decay_t<T>> underlying;
    std::vector<std::reference_wrapper<std::decay_t<T>>> ref_heap;

public:
    Heap() = default;

    template<typename... Args>
        requires std::constructible_from<decltype(underlying), Args...>
    Heap(Args&&...args) :
            underlying{std::forward<Args&&>(args)...},
            ref_heap{this->underlying | std::ranges::transform_view(&std::ref)} {
        std::ranges::make_heap(this->ref_heap);
    }

    constexpr std::strong_ordering operator<=>(const Heap& other) const {
        //The heap is all references, the underlying data is all that's "real"
        return this->underlying <=> other.underlying;
    }

    [[nodiscard]] constexpr auto data() const noexcept -> std::decay<T> * {
        return this->underlying.data();
    }

    [[nodiscard]] constexpr auto size() const noexcept -> size_t {
        return this->underlying.size();
    }

    [[nodiscard]] constexpr auto empty() const noexcept -> bool {
        return this->underlying.empty();
    }

    constexpr void push(const T& elem) {
        this->underlying.push_back(elem);
        this->ref_heap.push_back(std::ref(this->underlying.back()));
        std::ranges::push_heap(this->ref_heap);
    }
    constexpr void push(T&& elem) {
        this->underlying.push_back(std::forward<T>(elem));
        this->ref_heap.push_back(std::ref(this->underlying.back()));
        std::ranges::push_heap(this->ref_heap);
    }

    constexpr auto emplace(auto&&...args) {
        const auto ret = this->underlying.emplace_back(std::forward<decltype(args)>(args)...);
        this->ref_heap.push_back(std::ref(this->underlying.back()));
        std::ranges::push_heap(this->ref_heap);
        return ret;
    }
};

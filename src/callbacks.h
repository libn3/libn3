#pragma once

#include <functional>
#include <tuple>

namespace n3 {

//TODO: This is a one-time-only callback, do we want a second potential type for multi-call?
//TODO: This has issues with void/empty call args like the result of std::bind to collapse them all
template<typename... cArgs>
class callback {
    std::move_only_function<void(cArgs...)> mf;

public:
    //TODO: Verify requirements on std::move_only_function to prevent UB (like ensure Func type is constructible)
    template<typename F, typename... fArgs>
    constexpr callback(F&& func, fArgs&&...func_args) :
            mf{std::bind_front(std::forward<F>(func), std::forward<fArgs>(func_args)...)} {
    }
    template<typename F>
    constexpr callback(F&& func) : mf{std::forward<F>(func)} {
    }

    constexpr callback(const callback&) noexcept = delete;
    constexpr callback(callback&&) noexcept = default;

    constexpr callback& operator=(const callback&) noexcept = delete;
    constexpr callback& operator=(callback&&) noexcept = default;

    /*
     * The && at the end signifies that this is only callable by rvalue references of *this
     * It's a way to specify overload resolution based on reference type
     * And with it added explicitly, that means the lvalue reference call doesn't exist, so we get
     * a compile time error if the user doesn't std::move() the object when calling the callback
     */
    constexpr void operator()(cArgs&&...call_args) && noexcept(
            noexcept(this->mf(std::forward<cArgs>(call_args)...))) {
        std::invoke(std::move(this->mf), std::forward<cArgs>(call_args)...);
        //TODO: Would like to do something like empty out the function after usage by setting nullptr
        //Problem is, calling operator() on an empty std::move_only_function is UB, so I'd have to throw something myself
    }
};

template<>
class callback<void> {
    std::move_only_function<void()> mf;

public:
    //TODO: Verify requirements on std::move_only_function to prevent UB (like ensure Func type is constructible)
    template<typename F, typename... fArgs>
    constexpr callback(F&& func, fArgs&&...func_args) :
            mf{std::bind_front(std::forward<F>(func), std::forward<fArgs>(func_args)...)} {
    }
    template<typename F>
    constexpr callback(F&& func) : mf{std::forward<F>(func)} {
    }

    callback(const callback&) noexcept = delete;
    callback(callback&&) noexcept = default;

    callback& operator=(const callback&) noexcept = delete;
    callback& operator=(callback&&) noexcept = default;

    /*
     * The && at the end signifies that this is only callable by rvalue references of *this
     * It's a way to specify overload resolution based on reference type
     * And with it added explicitly, that means the lvalue reference call doesn't exist, so we get
     * a compile time error if the user doesn't std::move() the object when calling the callback
     */
    constexpr void operator()() && noexcept(noexcept(std::invoke(std::move(this->mf)))) {
        std::invoke(std::move(this->mf));
        //TODO: Would like to do something like empty out the function after usage by setting nullptr
        //Problem is, calling operator() on an empty std::move_only_function is UB, so I'd have to throw something myself
    }
};

} // namespace n3

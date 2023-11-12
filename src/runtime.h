#pragma once

#include <functional>
#include <tuple>

#include "buffer.h"
#include "epoll.h"
#include "error.h"

/**
 * Runtime is the meta glue layer between top level application usage and internals such as
 * epoll, raw TCP sockets, syscalls, and other details
 */

namespace n3::runtime {

template<typename... cArgs>
class callback {
    std::move_only_function<void(cArgs...)> mf;

public:
    template<typename F, typename... fArgs>
    callback(F&& func, fArgs&&...func_args) : mf{std::bind_front(func, func_args...)} {
    }

    callback(const callback&) = delete;
    callback(callback&&) noexcept = default;

    callback& operator=(const callback&) = delete;
    callback& operator=(callback&&) = default;

    /*
     * TODO: Callbacks may not necessarily be allowed to be called multiple times
     * We either need a "Multi-call callback" type to differentiate and/or find a way of changing
     * this object's lifecycle/implementation to handle single-use-only call semantics
     * Likely requires some nonsense with destructor calls, a trivial boolean flag is_executed,
     * and/or adding the && quality to the function (similar to const'ing a member function) to
     * force it to only exist for rvalue reference types
     * May or may not require std::move on the std::move_only_function as part of this, not sure
     */
    void operator()(cArgs&&...call_args) noexcept(this->mf()) {
        this->mf(call_args...);
    }
};

//TODO: Add configuration options through some init/builder/option struct pattern
//TODO: Add multithreaded runtime
//TODO: Naming?

class runtime_st {
    n3::epoll::epoll_ctx epoll;
    bool active;

    n3::PageBuffer read_buffer;

public:
    runtime_st() noexcept;

    void run();
};

} // namespace n3::runtime

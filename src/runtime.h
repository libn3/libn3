#pragma once

#include <tuple>

#include "epoll.h"
#include "error.h"

/**
 * Runtime is the meta glue layer between top level application usage and internals such as
 * epoll, raw TCP sockets, syscalls, and other details
 */

namespace n3 { namespace runtime {

    template<typename F, typename... Args>
    class callback {
        F func;
        std::tuple<Args...> args;

    public:
        callback();
    };

    //TODO: Add configuration options through some init/builder/option struct pattern
    //TODO: Add multithreaded runtime
    //TODO: Naming?

    class runtime_st {
        n3::epoll::epoll_ctx epoll;
        bool active;

    public:
        runtime_st() noexcept;

        void run();
    };

}} // namespace n3::runtime

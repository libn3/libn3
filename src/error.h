#pragma once

#include <cassert>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <optional>
#include <string_view>
#include <sys/types.h>
#include <utility>
#include <variant>

#include "buffer.h"

/*
 * TODO: I don't like this design for error codes, need a better one that's more ergonomic
 * Issue is that there's too much complexity being condensed down in a way that isn't reasonable
 * At any point in time, there are 3+ categories of errors that either myself or a user will care about:
 *  - libn3 internal errors (unintentional like std::bad_alloc being caught, or intentional like precondition EINVAL checks)
 *  - Native OS errors from syscalls (send returns EAGAIN, not "resource_unavailable_try_again", hard to write/understand)
 *  - "Alternative" errors (main one I've encountered is getaddrinfo GAI_* errors, but there could be others)
 *
 * I don't like having to juggle between them, try and condense one into the other, or rename more typical error codes
 * I don't want to give up checking for EAGAIN in library code on linux syscalls to know if things are still good or not
 * I also really don't want to use anything in <system_error>, because that is a Boost-style mess like iostreams are
 * I love std::expected, but what do I use as the error type in a way that makes sense?
 * I guess I could make a custom wrapper type and make it basically just an std::variant over the error types
 * Not sure, requires further design thought
 */

namespace n3::error {

enum class posix_error;
enum class gai_error;

struct posix_error_type {};
struct gai_error_type {};

struct posix_error_t {
    explicit posix_error_t() = default;
};
inline constexpr posix_error_t posix_error_arg{};

struct gai_error_t {
    explicit gai_error_t() = default;
};
inline constexpr gai_error_t gai_error_arg{};

class ErrorCode {
    //TODO: Do I need application errors, and do they need to be different from posix?
    std::variant<posix_error, gai_error> underlying;

public:
    constexpr ErrorCode() noexcept = default;

    //Constructor for anything that can be used to create the underlying variant
    constexpr ErrorCode(auto&&...args) noexcept(
            std::is_nothrow_constructible_v<decltype(underlying), decltype(args)...>) :
            underlying{std::forward<decltype(args)>(args)...} {
    }

    //Explicit error type constructor for posix errors using a trivial sentinel type
    template<std::integral T>
    constexpr ErrorCode(const posix_error_t, const T err_arg) noexcept :
            ErrorCode{posix_error{err_arg}} {
    }
    //Explicit error type constructor for getaddrinfo errors using a trivial sentinel type
    template<std::integral T>
    constexpr ErrorCode(const gai_error_t, const T err_arg) noexcept :
            ErrorCode{gai_error{err_arg}} {
    }

    constexpr ErrorCode(const ErrorCode&) = default;
    constexpr ErrorCode(ErrorCode&&) = default;

    constexpr ErrorCode& operator=(const ErrorCode&) = default;
    constexpr ErrorCode& operator=(ErrorCode&&) = default;

    constexpr auto operator<=>(const ErrorCode&) const = default;

    template<std::integral T>
    constexpr auto operator<=>(const T& arg) const noexcept {
        return this->underlying <=> arg;
    }

    constexpr const std::string_view what() const noexcept {
        auto&& handler = [](auto&& arg) -> const std::string_view {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, posix_error>) {
                return strerror(std::to_underlying(arg));
            } else if constexpr (std::is_same_v<T, gai_error>) {
                return gai_strerror(std::to_underlying(arg));
            } else {
                static_assert(std::false_type::value, "non-exhaustive visitor!");
                std::unreachable();
            }
        };
        return std::visit(std::move(handler), this->underlying);
    }
};

//Compiler errors to make sure an ErrorCode is as trivial as it should be
static_assert(std::is_trivially_copyable_v<ErrorCode>);
static_assert(std::is_trivially_copy_constructible_v<ErrorCode>);
static_assert(std::is_trivially_move_constructible_v<ErrorCode>);
static_assert(std::is_trivially_copy_assignable_v<ErrorCode>);
static_assert(std::is_trivially_move_assignable_v<ErrorCode>);
static_assert(std::is_trivially_destructible_v<ErrorCode>);
static_assert(std::is_nothrow_copy_constructible_v<ErrorCode>);
static_assert(std::is_nothrow_move_constructible_v<ErrorCode>);
static_assert(std::is_nothrow_copy_assignable_v<ErrorCode>);
static_assert(std::is_nothrow_move_assignable_v<ErrorCode>);

enum class posix_error {
    eafnosupport = EAFNOSUPPORT,
    eaddrinuse = EADDRINUSE,
    eaddrnotavail = EADDRNOTAVAIL,
    eisconn = EISCONN,
    e2big = E2BIG,
    edom = EDOM,
    efault = EFAULT,
    ebadf = EBADF,
    ebadmsg = EBADMSG,
    epipe = EPIPE,
    econnaborted = ECONNABORTED,
    ealready = EALREADY,
    econnrefused = ECONNREFUSED,
    econnreset = ECONNRESET,
    exdev = EXDEV,
    edestaddrreq = EDESTADDRREQ,
    ebusy = EBUSY,
    enotempty = ENOTEMPTY,
    enoexec = ENOEXEC,
    eexist = EEXIST,
    efbig = EFBIG,
    enametoolong = ENAMETOOLONG,
    enosys = ENOSYS,
    ehostunreach = EHOSTUNREACH,
    eidrm = EIDRM,
    eilseq = EILSEQ,
    enotty = ENOTTY,
    eintr = EINTR,
    einval = EINVAL,
    espipe = ESPIPE,
    eio = EIO,
    eisdir = EISDIR,
    emsgsize = EMSGSIZE,
    enetdown = ENETDOWN,
    enetreset = ENETRESET,
    enetunreach = ENETUNREACH,
    enobufs = ENOBUFS,
    echild = ECHILD,
    enolink = ENOLINK,
    enolck = ENOLCK,
    enomsg = ENOMSG,
    enoprotoopt = ENOPROTOOPT,
    enospc = ENOSPC,
    enxio = ENXIO,
    enodev = ENODEV,
    enoent = ENOENT,
    esrch = ESRCH,
    enotdir = ENOTDIR,
    enotsock = ENOTSOCK,
    enotconn = ENOTCONN,
    enomem = ENOMEM,
    ecanceled = ECANCELED,
    einprogress = EINPROGRESS,
    eperm = EPERM,
    enotsup = ENOTSUP,
    eownerdead = EOWNERDEAD,
    eacces = EACCES,
    eproto = EPROTO,
    eprotonosupport = EPROTONOSUPPORT,
    erofs = EROFS,
    edeadlk = EDEADLK,
    eagain = EAGAIN,
    erange = ERANGE,
    enotrecoverable = ENOTRECOVERABLE,
    etxtbsy = ETXTBSY,
    etimedout = ETIMEDOUT,
    enfile = ENFILE,
    emfile = EMFILE,
    emlink = EMLINK,
    eloop = ELOOP,
    eoverflow = EOVERFLOW,
    eprototype = EPROTOTYPE,
};

enum class gai_error {
    eai_addrfamily = EAI_ADDRFAMILY,
    eai_again = EAI_AGAIN,
    eai_badflags = EAI_BADFLAGS,
    eai_fail = EAI_FAIL,
    eai_family = EAI_FAMILY,
    eai_memory = EAI_MEMORY,
    eai_nodata = EAI_NODATA,
    eai_noname = EAI_NONAME,
    eai_service = EAI_SERVICE,
    eai_socktype = EAI_SOCKTYPE,
    eai_system = EAI_SYSTEM,
};

[[nodiscard]] constexpr ErrorCode get_error_code_from_errno(int errno_arg) noexcept {
    switch (errno_arg) {
        case EAFNOSUPPORT:
            return {posix_error::eafnosupport};
        case EADDRINUSE:
            return {posix_error::eaddrinuse};
        case EADDRNOTAVAIL:
            return {posix_error::eaddrnotavail};
        case EISCONN:
            return {posix_error::eisconn};
        case E2BIG:
            return {posix_error::e2big};
        case EDOM:
            return {posix_error::edom};
        case EFAULT:
            return {posix_error::efault};
        case EBADF:
            return {posix_error::ebadf};
        case EBADMSG:
            return {posix_error::ebadmsg};
        case EPIPE:
            return {posix_error::epipe};
        case ECONNABORTED:
            return {posix_error::econnaborted};
        case EALREADY:
            return {posix_error::ealready};
        case ECONNREFUSED:
            return {posix_error::econnrefused};
        case ECONNRESET:
            return {posix_error::econnreset};
        case EXDEV:
            return {posix_error::exdev};
        case EDESTADDRREQ:
            return {posix_error::edestaddrreq};
        case EBUSY:
            return {posix_error::ebusy};
        case ENOTEMPTY:
            return {posix_error::enotempty};
        case ENOEXEC:
            return {posix_error::enoexec};
        case EEXIST:
            return {posix_error::eexist};
        case EFBIG:
            return {posix_error::efbig};
        case ENAMETOOLONG:
            return {posix_error::enametoolong};
        case ENOSYS:
            return {posix_error::enosys};
        case EHOSTUNREACH:
            return {posix_error::ehostunreach};
        case EIDRM:
            return {posix_error::eidrm};
        case EILSEQ:
            return {posix_error::eilseq};
        case ENOTTY:
            return {posix_error::enotty};
        case EINTR:
            return {posix_error::eintr};
        case EINVAL:
            return {posix_error::einval};
        case ESPIPE:
            return {posix_error::espipe};
        case EIO:
            return {posix_error::eio};
        case EISDIR:
            return {posix_error::eisdir};
        case EMSGSIZE:
            return {posix_error::emsgsize};
        case ENETDOWN:
            return {posix_error::enetdown};
        case ENETRESET:
            return {posix_error::enetreset};
        case ENETUNREACH:
            return {posix_error::enetunreach};
        case ENOBUFS:
            return {posix_error::enobufs};
        case ECHILD:
            return {posix_error::echild};
        case ENOLINK:
            return {posix_error::enolink};
        case ENOLCK:
            return {posix_error::enolck};
        case ENOMSG:
            return {posix_error::enomsg};
        case ENOPROTOOPT:
            return {posix_error::enoprotoopt};
        case ENOSPC:
            return {posix_error::enospc};
        case ENXIO:
            return {posix_error::enxio};
        case ENODEV:
            return {posix_error::enodev};
        case ENOENT:
            return {posix_error::enoent};
        case ESRCH:
            return {posix_error::esrch};
        case ENOTDIR:
            return {posix_error::enotdir};
        case ENOTSOCK:
            return {posix_error::enotsock};
        case ENOTCONN:
            return {posix_error::enotconn};
        case ENOMEM:
            return {posix_error::enomem};
        case ENOTSUP:
            return {posix_error::enotsup};
            //case EOPNOTSUPP:
            static_assert(ENOTSUP == EOPNOTSUPP);
        case ECANCELED:
            return {posix_error::ecanceled};
        case EINPROGRESS:
            return {posix_error::einprogress};
        case EPERM:
            return {posix_error::eperm};
        case EOWNERDEAD:
            return {posix_error::eownerdead};
        case EACCES:
            return {posix_error::eacces};
        case EPROTO:
            return {posix_error::eproto};
        case EPROTONOSUPPORT:
            return {posix_error::eprotonosupport};
        case EROFS:
            return {posix_error::erofs};
        case EDEADLK:
            return {posix_error::edeadlk};
        case EAGAIN:
            return {posix_error::eagain};
            //case EWOULDBLOCK:
            static_assert(EAGAIN == EWOULDBLOCK);
        case ERANGE:
            return {posix_error::erange};
        case ENOTRECOVERABLE:
            return {posix_error::enotrecoverable};
        case ETXTBSY:
            return {posix_error::etxtbsy};
        case ETIMEDOUT:
            return {posix_error::etimedout};
        case ENFILE:
            return {posix_error::enfile};
        case EMFILE:
            return {posix_error::emfile};
        case EMLINK:
            return {posix_error::emlink};
        case ELOOP:
            return {posix_error::eloop};
        case EOVERFLOW:
            return {posix_error::eoverflow};
        case EPROTOTYPE:
            return {posix_error::eprototype};
        default:
            std::unreachable();
    }
}

[[nodiscard]] constexpr ErrorCode get_error_code_from_getaddrinfo_err(
        int addr_err, const int errno_arg) noexcept {
    switch (addr_err) {
        case EAI_ADDRFAMILY:
            return {gai_error::eai_addrfamily};
        case EAI_AGAIN:
            return {gai_error::eai_again};
        case EAI_BADFLAGS:
            return {gai_error::eai_badflags};
        case EAI_FAIL:
            return {gai_error::eai_fail};
        case EAI_FAMILY:
            return {gai_error::eai_family};
        case EAI_MEMORY:
            return {gai_error::eai_memory};
        case EAI_NODATA:
            return {gai_error::eai_nodata};
        case EAI_NONAME:
            return {gai_error::eai_noname};
        case EAI_SERVICE:
            return {gai_error::eai_service};
        case EAI_SOCKTYPE:
            return {gai_error::eai_socktype};
        case EAI_SYSTEM:
            return get_error_code_from_errno(errno_arg);
        default:
            std::unreachable();
    }
}

} // namespace n3::error

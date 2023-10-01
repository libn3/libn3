#pragma once

#include <cerrno>
#include <utility>

namespace n3 { namespace error {
    enum class code {
        address_family_not_supported = EAFNOSUPPORT,
        address_in_use = EADDRINUSE,
        address_not_available = EADDRNOTAVAIL,
        already_connected = EISCONN,
        argument_list_too_long = E2BIG,
        argument_out_of_domain = EDOM,
        bad_address = EFAULT,
        bad_file_descriptor = EBADF,
        bad_message = EBADMSG,
        broken_pipe = EPIPE,
        connection_aborted = ECONNABORTED,
        connection_already_in_progress = EALREADY,
        connection_refused = ECONNREFUSED,
        connection_reset = ECONNRESET,
        cross_device_link = EXDEV,
        destination_address_required = EDESTADDRREQ,
        device_or_resource_busy = EBUSY,
        directory_not_empty = ENOTEMPTY,
        executable_format_error = ENOEXEC,
        file_exists = EEXIST,
        file_too_large = EFBIG,
        filename_too_long = ENAMETOOLONG,
        function_not_supported = ENOSYS,
        host_unreachable = EHOSTUNREACH,
        identifier_removed = EIDRM,
        illegal_byte_sequence = EILSEQ,
        inappropriate_io_control_operation = ENOTTY,
        interrupted = EINTR,
        invalid_argument = EINVAL,
        invalid_seek = ESPIPE,
        io_error = EIO,
        is_a_directory = EISDIR,
        message_size = EMSGSIZE,
        network_down = ENETDOWN,
        network_reset = ENETRESET,
        network_unreachable = ENETUNREACH,
        no_buffer_space = ENOBUFS,
        no_child_process = ECHILD,
        no_link = ENOLINK,
        no_lock_available = ENOLCK,
        no_message = ENOMSG,
        no_protocol_option = ENOPROTOOPT,
        no_space_on_device = ENOSPC,
        no_such_device_or_address = ENXIO,
        no_such_device = ENODEV,
        no_such_file_or_directory = ENOENT,
        no_such_process = ESRCH,
        not_a_directory = ENOTDIR,
        not_a_socket = ENOTSOCK,
        not_connected = ENOTCONN,
        not_enough_memory = ENOMEM,
        operation_canceled = ECANCELED,
        operation_in_progress = EINPROGRESS,
        operation_not_permitted = EPERM,
        operation_not_supported = EOPNOTSUPP,
        owner_dead = EOWNERDEAD,
        permission_denied = EACCES,
        protocol_error = EPROTO,
        protocol_not_supported = EPROTONOSUPPORT,
        read_only_file_system = EROFS,
        resource_deadlock_would_occur = EDEADLK,
        resource_unavailable_try_again = EAGAIN,
        result_out_of_range = ERANGE,
        state_not_recoverable = ENOTRECOVERABLE,
        text_file_busy = ETXTBSY,
        timed_out = ETIMEDOUT,
        too_many_files_open_in_system = ENFILE,
        too_many_files_open = EMFILE,
        too_many_links = EMLINK,
        too_many_symbolic_link_levels = ELOOP,
        value_too_large = EOVERFLOW,
        wrong_protocol_type = EPROTOTYPE,
    };

    [[nodiscard]] constexpr code get_error_code_from_errno(int errno_arg) noexcept {
        switch (errno_arg) {
            case EAFNOSUPPORT:
                return code::address_family_not_supported;
            case EADDRINUSE:
                return code::address_in_use;
            case EADDRNOTAVAIL:
                return code::address_not_available;
            case EISCONN:
                return code::already_connected;
            case E2BIG:
                return code::argument_list_too_long;
            case EDOM:
                return code::argument_out_of_domain;
            case EFAULT:
                return code::bad_address;
            case EBADF:
                return code::bad_file_descriptor;
            case EBADMSG:
                return code::bad_message;
            case EPIPE:
                return code::broken_pipe;
            case ECONNABORTED:
                return code::connection_aborted;
            case EALREADY:
                return code::connection_already_in_progress;
            case ECONNREFUSED:
                return code::connection_refused;
            case ECONNRESET:
                return code::connection_reset;
            case EXDEV:
                return code::cross_device_link;
            case EDESTADDRREQ:
                return code::destination_address_required;
            case EBUSY:
                return code::device_or_resource_busy;
            case ENOTEMPTY:
                return code::directory_not_empty;
            case ENOEXEC:
                return code::executable_format_error;
            case EEXIST:
                return code::file_exists;
            case EFBIG:
                return code::file_too_large;
            case ENAMETOOLONG:
                return code::filename_too_long;
            case ENOSYS:
                return code::function_not_supported;
            case EHOSTUNREACH:
                return code::host_unreachable;
            case EIDRM:
                return code::identifier_removed;
            case EILSEQ:
                return code::illegal_byte_sequence;
            case ENOTTY:
                return code::inappropriate_io_control_operation;
            case EINTR:
                return code::interrupted;
            case EINVAL:
                return code::invalid_argument;
            case ESPIPE:
                return code::invalid_seek;
            case EIO:
                return code::io_error;
            case EISDIR:
                return code::is_a_directory;
            case EMSGSIZE:
                return code::message_size;
            case ENETDOWN:
                return code::network_down;
            case ENETRESET:
                return code::network_reset;
            case ENETUNREACH:
                return code::network_unreachable;
            case ENOBUFS:
                return code::no_buffer_space;
            case ECHILD:
                return code::no_child_process;
            case ENOLINK:
                return code::no_link;
            case ENOLCK:
                return code::no_lock_available;
            case ENOMSG:
                return code::no_message;
            case ENOPROTOOPT:
                return code::no_protocol_option;
            case ENOSPC:
                return code::no_space_on_device;
            case ENXIO:
                return code::no_such_device_or_address;
            case ENODEV:
                return code::no_such_device;
            case ENOENT:
                return code::no_such_file_or_directory;
            case ESRCH:
                return code::no_such_process;
            case ENOTDIR:
                return code::not_a_directory;
            case ENOTSOCK:
                return code::not_a_socket;
            case ENOTCONN:
                return code::not_connected;
            case ENOMEM:
                return code::not_enough_memory;
            case ENOTSUP:
                //case EOPNOTSUPP:
                static_assert(ENOTSUP == EOPNOTSUPP);
                return code::operation_not_supported;
            case ECANCELED:
                return code::operation_canceled;
            case EINPROGRESS:
                return code::operation_in_progress;
            case EPERM:
                return code::operation_not_permitted;
            case EOWNERDEAD:
                return code::owner_dead;
            case EACCES:
                return code::permission_denied;
            case EPROTO:
                return code::protocol_error;
            case EPROTONOSUPPORT:
                return code::protocol_not_supported;
            case EROFS:
                return code::read_only_file_system;
            case EDEADLK:
                return code::resource_deadlock_would_occur;
            case EAGAIN:
                //case EWOULDBLOCK:
                static_assert(EAGAIN == EWOULDBLOCK);
                return code::resource_unavailable_try_again;
            case ERANGE:
                return code::result_out_of_range;
            case ENOTRECOVERABLE:
                return code::state_not_recoverable;
            case ETXTBSY:
                return code::text_file_busy;
            case ETIMEDOUT:
                return code::timed_out;
            case ENFILE:
                return code::too_many_files_open_in_system;
            case EMFILE:
                return code::too_many_files_open;
            case EMLINK:
                return code::too_many_links;
            case ELOOP:
                return code::too_many_symbolic_link_levels;
            case EOVERFLOW:
                return code::value_too_large;
            case EPROTOTYPE:
                return code::wrong_protocol_type;
            default:
                std::unreachable();
        }
    }
}} // namespace n3::error

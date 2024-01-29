#include <memory>
#include <mutex>
#include <unistd.h>
#include <utility>
#include <vector>

#include "syscalls.h"

namespace n3 {

[[nodiscard]] size_t GetPageSize() {
    static size_t PAGE_SIZE;
    static std::once_flag page_size_init_flag;

    std::call_once(page_size_init_flag,
            [&] { PAGE_SIZE = n3::linux::sysconf(_SC_PAGESIZE).value_or(4096); });
    return PAGE_SIZE;
}

} // namespace n3

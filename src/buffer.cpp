#include <memory>
#include <mutex>
#include <unistd.h>
#include <utility>
#include <vector>

#include "buffer.h"

#include "syscalls.h"

namespace n3 {

class PageSize {
    static size_t PAGE_SIZE;
    static std::once_flag page_size_init_flag;

public:
    static size_t get() {
        std::call_once(page_size_init_flag, [&] {
            const auto ret = n3::linux::sysconf(_SC_PAGESIZE);
            PAGE_SIZE = ret.value_or(4096);
        });
        return PAGE_SIZE;
    }
};

OwningBuffer::OwningBuffer() : data{} {
}
OwningBuffer::OwningBuffer(std::span<std::byte> init_data) :
        data{init_data.begin(), init_data.end()} {
}
OwningBuffer::OwningBuffer(std::vector<std::byte>&& init_data) : data{init_data} {
}

PageBuffer::PageBuffer() :
        page_size{PageSize::get()},
        underlying{[] {
            const auto ps = PageSize::get();
            return std::unique_ptr<std::byte[]>(reinterpret_cast<std::byte *>(
                    ::operator new[](ps, static_cast<std::align_val_t>(ps))));
        }()} {
}

} // namespace n3

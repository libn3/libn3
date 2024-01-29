#include <memory>
#include <mutex>
#include <unistd.h>
#include <utility>
#include <vector>

#include "buffer.h"

#include "page_size.h"
#include "syscalls.h"

namespace n3 {

OwningBuffer::OwningBuffer() : data{} {
}
OwningBuffer::OwningBuffer(std::span<std::byte> init_data) :
        data{init_data.begin(), init_data.end()} {
}
OwningBuffer::OwningBuffer(std::vector<std::byte>&& init_data) : data{init_data} {
}

//TODO: Use mmap to guarantee a page instead of doing so indirectly via operator new allocation
PageBuffer::PageBuffer() :
        page_size{GetPageSize()},
        underlying{[] {
            const auto ps = GetPageSize();
            return std::unique_ptr<std::byte[]>(reinterpret_cast<std::byte *>(
                    ::operator new[](ps, static_cast<std::align_val_t>(ps))));
        }()} {
}

} // namespace n3

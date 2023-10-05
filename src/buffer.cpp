#include <memory>
#include <unistd.h>
#include <utility>
#include <vector>

#include "buffer.h"

namespace ns {

OwningBuffer::OwningBuffer() : data{} {
}
OwningBuffer::OwningBuffer(std::span<std::byte> init_data) :
        data{init_data.begin(), init_data.end()} {
}
OwningBuffer::OwningBuffer(std::vector<std::byte>&& init_data) : data{init_data} {
}

} // namespace ns

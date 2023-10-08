#pragma once

#include <memory>
#include <span>
#include <utility>
#include <vector>

namespace ns {

class OwningBuffer {
    std::vector<std::byte> data;

public:
    OwningBuffer();
    OwningBuffer(std::span<std::byte> init_data);
    OwningBuffer(std::vector<std::byte>&& init_data);
};

class RefBuffer {};

} // namespace ns

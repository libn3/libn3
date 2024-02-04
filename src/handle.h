#pragma once

#include "ownership.h"

namespace n3 {

using Handle = int;

class OwnedHandle {
    const MoveOnly<Handle> fd;

public:
    explicit OwnedHandle(const Handle fd_arg) noexcept;
    ~OwnedHandle();

    //Conversion operator to treat this as a plain handle type
    [[nodiscard]] constexpr operator Handle() const noexcept {
        return this->fd;
    }
};

} // namespace n3

#pragma once

namespace n3 {

using Handle = int;

class OwnedHandle {
    const Handle fd;

public:
    explicit OwnedHandle(const Handle fd_arg) noexcept;
    ~OwnedHandle() noexcept;

    constexpr OwnedHandle(const OwnedHandle&) noexcept = delete;
    constexpr OwnedHandle(OwnedHandle&&) noexcept = default;

    constexpr OwnedHandle& operator=(const OwnedHandle&) noexcept = delete;
    constexpr OwnedHandle& operator=(OwnedHandle&&) noexcept = default;

    //Conversion operator to treat this as a plain handle type
    [[nodiscard]] constexpr operator Handle() const noexcept {
        return this->fd;
    }
};

} // namespace n3

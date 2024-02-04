#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

#include "ownership.h"

namespace n3 {

//Some static asserts to verify that the MoveOnly type works with some reasonable types
static_assert(std::movable<MoveOnly<int>>);
static_assert(std::movable<MoveOnly<std::unique_ptr<int>>>);
static_assert(std::movable<MoveOnly<std::shared_ptr<int>>>);
static_assert(std::movable<MoveOnly<std::function<void(int)>>>);
static_assert(std::movable<MoveOnly<void (*)(int)>>);
static_assert(std::movable<MoveOnly<std::vector<int>>>);
static_assert(std::movable<MoveOnly<std::optional<int>>>);

}; // namespace n3

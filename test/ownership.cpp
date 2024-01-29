#include <catch2/catch_test_macros.hpp>

#include "ownership.h"

struct aggregate {
    int x;
    int y;
};

TEST_CASE("MoveOnly data access") {
    REQUIRE(*n3::MoveOnly<int>(1) == 1);

    REQUIRE(n3::MoveOnly<aggregate>(3, 4)->x == 3);
}

TEST_CASE("MoveOnly moves trivial resources correctly") {
    n3::MoveOnly<int> data{1};

    REQUIRE(data);
    REQUIRE(*data == 1);

    n3::MoveOnly<int> move_data{std::move(data)};
    REQUIRE(!data);
    REQUIRE(move_data);
    REQUIRE(*move_data == 1);
    *move_data = 2;
    REQUIRE(*move_data == 2);
}

struct noncopyable {
    std::unique_ptr<int> ptr;
};
static_assert(std::movable<noncopyable>);

TEST_CASE("MoveOnly moves noncopyable resources correctly") {
    n3::MoveOnly<noncopyable> data{std::make_unique<int>(1)};

    REQUIRE(data);
    REQUIRE(data->ptr);
    REQUIRE(*data->ptr == 1);

    n3::MoveOnly<noncopyable> move_data{std::move(data)};
    REQUIRE(!data);
    REQUIRE(move_data->ptr != nullptr);
    REQUIRE(*move_data->ptr == 1);
}

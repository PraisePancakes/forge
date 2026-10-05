#include <forge/forge.hpp>
#include <iostream>
#include <vector>

#include "doctest.h"

using namespace forge;

namespace {
using pool = forge::basic_sparse_set<forge::entity>;
using traits = forge::entity_traits<forge::entity>;

constexpr auto make_entity(std::uint64_t id, std::uint64_t version = 0) {
    return traits::construct(
        static_cast<traits::entity_type>(id),
        static_cast<traits::version_type>(version));
}
}  // namespace

TEST_CASE("sparse_set_iterator: dereference") {
    pool entities;

    const auto e0 = make_entity(0);
    const auto e1 = make_entity(1);
    const auto e2 = make_entity(2);

    entities.push(e0);
    entities.push(e1);
    entities.push(e2);

    auto it = entities.begin();

    CHECK(*it == e0);
}

TEST_CASE("sparse_set_iterator: increment") {
    pool entities;

    const auto e0 = make_entity(0);
    const auto e1 = make_entity(1);
    const auto e2 = make_entity(2);

    entities.push(e0);
    entities.push(e1);
    entities.push(e2);

    auto it = entities.begin();

    CHECK(*it == e0);

    ++it;
    CHECK(*it == e1);

    ++it;
    CHECK(*it == e2);

    ++it;
    CHECK(it == entities.end());
}

TEST_CASE("sparse_set_iterator: post increment") {
    pool entities;

    const auto e0 = make_entity(0);
    const auto e1 = make_entity(1);
    const auto e2 = make_entity(2);

    entities.push(e0);
    entities.push(e1);
    entities.push(e2);

    auto it = entities.begin();

    auto old = it++;

    CHECK(*old == e0);
    CHECK(*it == e1);

    old = it++;

    CHECK(*old == e1);
    CHECK(*it == e2);
}

TEST_CASE("sparse_set_iterator: random access") {
    pool entities;

    const auto e0 = make_entity(0);
    const auto e1 = make_entity(1);
    const auto e2 = make_entity(2);

    entities.push(e0);
    entities.push(e1);
    entities.push(e2);

    auto it = entities.begin();

    CHECK(it[0] == e0);
    CHECK(it[1] == e1);
    CHECK(it[2] == e2);

    CHECK(*(it + 0) == e0);
    CHECK(*(it + 1) == e1);
    CHECK(*(it + 2) == e2);

    it += 2;
    CHECK(*it == e2);

    it -= 1;
    CHECK(*it == e1);

    CHECK(*(it + 1) == e2);
    CHECK(it + 2 == entities.end());
}

TEST_CASE("sparse_set_iterator: decrement") {
    pool entities;

    const auto e0 = make_entity(0);
    const auto e1 = make_entity(1);
    const auto e2 = make_entity(2);

    entities.push(e0);
    entities.push(e1);
    entities.push(e2);

    auto it = entities.end();

    --it;
    CHECK(*it == e2);

    --it;
    CHECK(*it == e1);

    --it;
    CHECK(*it == e0);
}

TEST_CASE("sparse_set_iterator: equality") {
    pool entities;

    const auto e0 = make_entity(0);
    const auto e1 = make_entity(1);

    entities.push(e0);
    entities.push(e1);

    auto a = entities.begin();
    auto b = entities.begin();

    CHECK(a == b);
    CHECK(!(a != b));

    ++b;

    CHECK(a != b);
    CHECK(!(a == b));

    CHECK(b == entities.begin() + 1);
}

TEST_CASE("sparse_set_iterator: arrow") {
    pool entities;

    const auto e0 = make_entity(0);

    entities.push(e0);

    auto it = entities.begin();

    CHECK(*it == e0);
}

TEST_CASE("sparse_set_iterator: empty pool") {
    pool entities;

    auto it = entities.begin();
    auto end = entities.end();

    CHECK(it == end);
    CHECK(!(it != end));
    CHECK(entities.size() == 0);
}

TEST_CASE("sparse_set_iterator: iteration") {
    pool entities;

    const auto e0 = make_entity(0);
    const auto e1 = make_entity(1);
    const auto e2 = make_entity(2);

    entities.push(e0);
    entities.push(e1);
    entities.push(e2);

    auto it = entities.begin();
    auto end = entities.end();

    std::vector<forge::entity> result;

    for (; it != end; ++it) {
        result.push_back(*it);
    }

    REQUIRE(result.size() == 3);
    CHECK(result[0] == e0);
    CHECK(result[1] == e1);
    CHECK(result[2] == e2);
}

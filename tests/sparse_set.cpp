#include <forge/forge.hpp>

#include "doctest.h"

using u64_t = forge::entity_traits<std::uint64_t>;
using u32_t = forge::entity_traits<std::uint32_t>;

TEST_CASE("sparse set starts empty") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(1, 0);

    CHECK_FALSE(set.contains(e0));
    CHECK_FALSE(set.contains(e1));
}

TEST_CASE("sparse set can insert an entity") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e = u64_t::construct(0, 0);

    set.push(e);

    CHECK(set.contains(e));
}

TEST_CASE("sparse set can contain multiple entities") {
    forge::basic_sparse_set<std::uint64_t> set;
    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(1, 0);
    const auto e2 = u64_t::construct(2, 0);

    set.push(e0);
    set.push(e1);
    set.push(e2);

    CHECK(set.contains(e0));
    CHECK(set.contains(e1));
    CHECK(set.contains(e2));
}

TEST_CASE("sparse set distinguishes entity generations") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(0, 1);

    set.push(e0);

    CHECK(set.contains(e0));
    CHECK_FALSE(set.contains(e1));
}

TEST_CASE("sparse set can remove an entity") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(1, 0);

    set.push(e0);
    set.push(e1);

    CHECK(set.contains(e0));
    CHECK(set.contains(e1));

    set.remove(e0);
    CHECK_FALSE(set.contains(e0));
    CHECK(set.contains(e1));
}

TEST_CASE("removing the only entity works") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e = u64_t::construct(0, 0);

    set.push(e);
    CHECK(set.contains(e));

    set.remove(e);

    CHECK_FALSE(set.contains(e));
}

TEST_CASE("removing an entity preserves the remaining entities") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(1, 0);
    const auto e2 = u64_t::construct(2, 0);

    set.push(e0);
    set.push(e1);
    set.push(e2);

    set.remove(e1);

    CHECK(set.contains(e0));
    CHECK_FALSE(set.contains(e1));
    CHECK(set.contains(e2));
}

TEST_CASE("removing the first entity preserves the last entity") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(1, 0);
    const auto e2 = u64_t::construct(2, 0);

    set.push(e0);
    set.push(e1);
    set.push(e2);

    set.remove(e0);

    CHECK_FALSE(set.contains(e0));
    CHECK(set.contains(e1));
    CHECK(set.contains(e2));
}

TEST_CASE("removing the last entity preserves the earlier entities") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(1, 0);
    const auto e2 = u64_t::construct(2, 0);

    set.push(e0);
    set.push(e1);
    set.push(e2);

    set.remove(e2);

    CHECK(set.contains(e0));
    CHECK(set.contains(e1));
    CHECK_FALSE(set.contains(e2));
}

TEST_CASE("sparse set handles sparse entity identifiers") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e256 = u64_t::construct(256, 0);
    const auto e1024 = u64_t::construct(1024, 0);

    set.push(e0);
    set.push(e256);
    set.push(e1024);

    CHECK(set.contains(e0));
    CHECK(set.contains(e256));
    CHECK(set.contains(e1024));
}

TEST_CASE("sparse set handles entities crossing page boundaries") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e255 = u64_t::construct(255, 0);
    const auto e256 = u64_t::construct(256, 0);
    const auto e257 = u64_t::construct(257, 0);

    set.push(e255);
    set.push(e256);
    set.push(e257);

    CHECK(set.contains(e255));
    CHECK(set.contains(e256));
    CHECK(set.contains(e257));

    set.remove(e256);

    CHECK(set.contains(e255));
    CHECK_FALSE(set.contains(e256));
    CHECK(set.contains(e257));
}

TEST_CASE("sparse set handles entities far apart") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e10000 = u64_t::construct(10000, 0);

    set.push(e0);
    set.push(e10000);

    CHECK(set.contains(e0));
    CHECK(set.contains(e10000));

    set.remove(e0);

    CHECK_FALSE(set.contains(e0));
    CHECK(set.contains(e10000));
}

TEST_CASE("sparse set can reuse an identifier with a new generation") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(0, 1);

    set.push(e0);
    CHECK(set.contains(e0));
    CHECK_FALSE(set.contains(e1));

    set.remove(e0);

    CHECK_FALSE(set.contains(e0));
    CHECK_FALSE(set.contains(e1));

    set.push(e1);

    CHECK_FALSE(set.contains(e0));
    CHECK(set.contains(e1));
}

TEST_CASE("sparse set supports uint32 entities") {
    forge::basic_sparse_set<std::uint32_t> set;

    const auto e0 = u32_t::construct(0, 0);
    const auto e1 = u32_t::construct(1, 0);
    const auto e2 = u32_t::construct(2, 5);

    set.push(e0);
    set.push(e1);
    set.push(e2);

    CHECK(set.contains(e0));
    CHECK(set.contains(e1));
    CHECK(set.contains(e2));

    CHECK_FALSE(set.contains(u32_t::construct(2, 4)));
}
TEST_CASE("sparse set operator[] returns the entity at its sparse index") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(1, 0);
    const auto e2 = u64_t::construct(2, 0);

    set.push(e0);
    set.push(e1);
    set.push(e2);

    CHECK(set[e0] == e0);
    CHECK(set[e1] == e1);
    CHECK(set[e2] == e2);
}
TEST_CASE("sparse set index_of returns the entity's dense index") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(1, 0);
    const auto e2 = u64_t::construct(2, 0);

    set.push(e0);
    set.push(e1);
    set.push(e2);

    CHECK(set.index_of(e0) == 0);
    CHECK(set.index_of(e1) == 1);
    CHECK(set.index_of(e2) == 2);
}

TEST_CASE("sparse set index_of follows entities after removal") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(1, 0);
    const auto e2 = u64_t::construct(2, 0);

    set.push(e0);
    set.push(e1);
    set.push(e2);

    set.remove(e0);

    // e2 was moved from dense index 2 to dense index 0.
    CHECK(set.index_of(e2) == 0);
    CHECK(set.index_of(e1) == 1);
}

TEST_CASE("sparse set index_of works across sparse pages") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e253 = u64_t::construct(253, 0);
    const auto e254 = u64_t::construct(254, 0);
    const auto e1024 = u64_t::construct(1024, 0);

    set.push(e253);
    set.push(e254);
    set.push(e1024);
    CHECK(set.index_of(e253) == 0);
    CHECK(set.index_of(e254) == 1);
    CHECK(set.index_of(e1024) == 2);
}

//./build/tests/forge_tests -tc="basic_sparse_set: contains multiple entities"
TEST_CASE("basic_sparse_set: contains multiple entities") {
    using pool = forge::basic_sparse_set<forge::entity>;

    pool entities;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(1, 0);
    const auto e2 = u64_t::construct(2, 0);

    entities.push(e0);
    entities.push(e1);
    entities.push(e2);

    CHECK(entities.contains(e0));
    CHECK(entities.contains(e1));
    CHECK(entities.contains(e2));
}

TEST_CASE("sparse set operator[] follows entities after removal") {
    forge::basic_sparse_set<std::uint64_t> set;

    const auto e0 = u64_t::construct(0, 0);
    const auto e1 = u64_t::construct(1, 0);
    const auto e2 = u64_t::construct(2, 0);

    set.push(e0);
    set.push(e1);
    set.push(e2);

    set.remove(e0);

    // e2 was moved into e0's old dense position.
    CHECK(set[e1] == e1);
    CHECK(set[e2] == e2);
    CHECK_FALSE(set.contains(e0));
}
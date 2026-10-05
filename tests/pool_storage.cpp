#include <forge/forge.hpp>
#include <string>

#include "doctest.h"

using u64_t = forge::entity_traits<std::uint64_t>;

TEST_SUITE("pool storage") {
    TEST_CASE("pool storage starts empty") {
        forge::storage::pool_storage<std::uint64_t, int> storage;

        const auto e0 = u64_t::construct(0, 0);

        CHECK_FALSE(storage.contains(e0));
    }

    TEST_CASE("pool storage can emplace a component") {
        forge::storage::pool_storage<std::uint64_t, int> storage;

        const auto e0 = u64_t::construct(0, 0);

        storage.emplace(e0, 42);

        CHECK(storage.contains(e0));
        CHECK(storage[e0] == 42);
    }

    TEST_CASE("pool storage stores components alongside entities") {
        forge::storage::pool_storage<std::uint64_t, int> storage;

        const auto e0 = u64_t::construct(0, 0);
        const auto e1 = u64_t::construct(1, 0);
        const auto e2 = u64_t::construct(2, 0);

        storage.emplace(e0, 10);
        storage.emplace(e1, 20);
        storage.emplace(e2, 30);

        CHECK(storage.contains(e0));
        CHECK(storage.contains(e1));
        CHECK(storage.contains(e2));

        CHECK(storage[e0] == 10);
        CHECK(storage.get(e1) == 20);
        CHECK(storage.get(e2) == 30);
    }

    TEST_CASE("pool storage can replace a component") {
        forge::storage::pool_storage<std::uint64_t, int> storage;

        const auto e0 = u64_t::construct(0, 0);

        storage.emplace(e0, 42);

        CHECK(storage.get(e0) == 42);

        storage.replace(e0, 100);

        CHECK(storage.contains(e0));
        CHECK(storage.get(e0) == 100);
    }

    TEST_CASE("pool storage can remove a component") {
        forge::storage::pool_storage<std::uint64_t, int> storage;

        const auto e0 = u64_t::construct(0, 0);
        const auto e1 = u64_t::construct(1, 0);

        storage.emplace(e0, 10);
        storage.emplace(e1, 20);

        storage.remove(e0);

        CHECK_FALSE(storage.contains(e0));
        CHECK(storage.contains(e1));
        CHECK(storage.get(e1) == 20);
    }

    TEST_CASE("pool storage keeps components associated after swap and pop") {
        forge::storage::pool_storage<std::uint64_t, int> storage;

        const auto e0 = u64_t::construct(0, 0);
        const auto e1 = u64_t::construct(1, 0);
        const auto e2 = u64_t::construct(2, 0);

        storage.emplace(e0, 100);
        storage.emplace(e1, 200);
        storage.emplace(e2, 300);

        storage.remove(e0);

        CHECK_FALSE(storage.contains(e0));

        CHECK(storage.contains(e1));
        CHECK(storage.contains(e2));

        CHECK(storage.get(e1) == 200);
        CHECK(storage.get(e2) == 300);
    }

    TEST_CASE("pool storage can remove the last component") {
        forge::storage::pool_storage<std::uint64_t, int> storage;

        const auto e0 = u64_t::construct(0, 0);
        const auto e1 = u64_t::construct(1, 0);

        storage.emplace(e0, 10);
        storage.emplace(e1, 20);

        storage.remove(e1);

        CHECK(storage.contains(e0));
        CHECK_FALSE(storage.contains(e1));

        CHECK(storage.get(e0) == 10);
    }

    TEST_CASE("pool storage can remove the only component") {
        forge::storage::pool_storage<std::uint64_t, int> storage;

        const auto e0 = u64_t::construct(0, 0);

        storage.emplace(e0, 42);

        CHECK(storage.contains(e0));

        storage.remove(e0);

        CHECK_FALSE(storage.contains(e0));
    }

    TEST_CASE("pool storage preserves entity generations") {
        forge::storage::pool_storage<std::uint64_t, int> storage;

        const auto e0_v0 = u64_t::construct(0, 0);
        const auto e0_v1 = u64_t::construct(0, 1);

        storage.emplace(e0_v0, 42);

        CHECK(storage.contains(e0_v0));
        CHECK_FALSE(storage.contains(e0_v1));

        storage.remove(e0_v0);

        CHECK_FALSE(storage.contains(e0_v0));
        CHECK_FALSE(storage.contains(e0_v1));

        storage.emplace(e0_v1, 100);

        CHECK_FALSE(storage.contains(e0_v0));
        CHECK(storage.contains(e0_v1));
        CHECK(storage.get(e0_v1) == 100);
    }

    TEST_CASE("pool storage can remove a non-existent entity") {
        forge::storage::pool_storage<std::uint64_t, int> storage;

        const auto e0 = u64_t::construct(0, 0);

        CHECK_FALSE(storage.contains(e0));

        storage.remove(e0);

        CHECK_FALSE(storage.contains(e0));
    }
}
#include <doctest/doctest.h>

#include <forge/forge.hpp>
#include <tuple>

TEST_SUITE("algorithms::containers") {
    using entity = forge::entity;
    using generator = forge::generation::generator<entity>;

    using int_pool = forge::storage::sparse_set<int, entity>;
    using char_pool = forge::storage::sparse_set<char, entity>;
    using float_pool = forge::storage::sparse_set<float, entity>;

    using pools_type = std::tuple<
        int_pool,
        char_pool,
        float_pool>;

    TEST_CASE("get_driving_index finds smallest non-empty pool") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();
        auto e2 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 1);
        std::get<0>(pools).emplace(e1, 2);
        std::get<0>(pools).emplace(e2, 3);

        std::get<1>(pools).emplace(e0, 'a');

        std::get<2>(pools).emplace(e0, 4.0f);
        std::get<2>(pools).emplace(e1, 5.0f);

        CHECK(forge::algorithms::containers::get_driving_index(pools) == 1);
    }

    TEST_CASE("get_driving_index finds first smallest pool") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 1);
        std::get<0>(pools).emplace(e1, 2);

        std::get<1>(pools).emplace(e0, 'a');
        std::get<1>(pools).emplace(e1, 'b');

        auto e2 = gen.make();
        std::get<2>(pools).emplace(e2, 4.0f);

        CHECK(forge::algorithms::containers::get_driving_index(pools) == 2);
    }

    TEST_CASE("get_driving_index handles equal sized pools") {
        generator gen;

        auto e0 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 1);
        std::get<1>(pools).emplace(e0, 'a');
        std::get<2>(pools).emplace(e0, 4.0f);

        // First pool of the minimum size wins.
        CHECK(forge::algorithms::containers::get_driving_index(pools) == 0);
    }

    TEST_CASE("get_driving_index ignores empty pools") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();
        auto e2 = gen.make();

        pools_type pools;

        // int pool intentionally empty.

        std::get<1>(pools).emplace(e0, 'a');
        std::get<1>(pools).emplace(e1, 'b');
        std::get<1>(pools).emplace(e2, 'c');

        std::get<2>(pools).emplace(e0, 4.0f);
        std::get<2>(pools).emplace(e1, 5.0f);

        CHECK(forge::algorithms::containers::get_driving_index(pools) == 2);
    }

    TEST_CASE("get_driving_index with all empty pools") {
        pools_type pools;

        // Current implementation leaves the index at zero.
        CHECK(forge::algorithms::containers::get_driving_index(pools) == 0);
    }

    TEST_CASE("subset_of selects requested pool types") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 1);
        std::get<0>(pools).emplace(e1, 2);

        std::get<1>(pools).emplace(e0, 'a');

        std::get<2>(pools).emplace(e1, 4.0f);

        auto subset = forge::algorithms::containers::subset_of<decltype(pools), int, float>(pools);

        CHECK(std::get<0>(subset).size() == 2);
        CHECK(std::get<1>(subset).size() == 1);

        CHECK(std::get<0>(subset).contains(e0));
        CHECK(std::get<0>(subset).contains(e1));

        CHECK(std::get<1>(subset).contains(e1));
    }

    TEST_CASE("subset_of preserves references") {
        generator gen;

        auto e0 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 42);

        auto subset = forge::algorithms::containers::subset_of<decltype(pools), int>(pools);

        std::get<0>(subset).get(e0) = 100;

        CHECK(std::get<0>(pools).get(e0) == 100);

        static_assert(std::is_same_v<decltype(std::get<0>(subset)), int_pool&>);
    }

    TEST_CASE("subset_of preserves requested order") {
        generator gen;

        auto e0 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 42);
        std::get<1>(pools).emplace(e0, 'x');
        std::get<2>(pools).emplace(e0, 2.5f);

        auto subset = forge::algorithms::containers::subset_of<decltype(pools), float, int>(pools);

        CHECK(std::get<0>(subset).contains(e0));
        CHECK(std::get<1>(subset).contains(e0));

        CHECK(std::get<0>(subset).get(e0) == 2.5f);
        CHECK(std::get<1>(subset).get(e0) == 42);

        static_assert(std::is_same_v<decltype(std::get<0>(subset)), float_pool&>);
        static_assert(std::is_same_v<decltype(std::get<1>(subset)), int_pool&>);
    }

    TEST_CASE("subset_of works with sparse sets") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 10);
        std::get<2>(pools).emplace(e1, 20.0f);

        auto subset = forge::algorithms::containers::subset_of<decltype(pools), int, float>(pools);

        CHECK(std::get<0>(subset).contains(e0));
        CHECK(std::get<1>(subset).contains(e1));
    }

    TEST_CASE("contains_in_every returns true only when entity is in every pool") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();
        auto e2 = gen.make();

        pools_type pools;

        // e0 exists in every pool.
        std::get<0>(pools).emplace(e0, 10);
        std::get<1>(pools).emplace(e0, 'a');
        std::get<2>(pools).emplace(e0, 1.0f);

        // e1 exists only in int and char.
        std::get<0>(pools).emplace(e1, 20);
        std::get<1>(pools).emplace(e1, 'b');

        // e2 exists only in int.
        std::get<0>(pools).emplace(e2, 30);

        CHECK(forge::algorithms::containers::contains_in_every(e0, pools));
        CHECK_FALSE(forge::algorithms::containers::contains_in_every(e1, pools));
        CHECK_FALSE(forge::algorithms::containers::contains_in_every(e2, pools));
    }

    TEST_CASE("contains_in_every returns false for unknown entity") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 10);
        std::get<1>(pools).emplace(e0, 'a');
        std::get<2>(pools).emplace(e0, 1.0f);

        CHECK_FALSE(forge::algorithms::containers::contains_in_every(e1, pools));
    }

    TEST_CASE("contains_in_none returns true when entity is absent from every pool") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();
        auto e2 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 10);
        std::get<1>(pools).emplace(e1, 'a');

        CHECK(forge::algorithms::containers::contains_in_none(e2, pools));
        CHECK_FALSE(forge::algorithms::containers::contains_in_none(e0, pools));

        CHECK_FALSE(forge::algorithms::containers::contains_in_none(e1, pools));
    }

    TEST_CASE("contains_in_none returns false if entity exists in any pool") {
        generator gen;

        auto e0 = gen.make();

        pools_type pools;

        std::get<2>(pools).emplace(e0, 42.0f);

        CHECK_FALSE(forge::algorithms::containers::contains_in_none(e0, pools));
    }

    TEST_CASE("pool_of returns mutable component for mutable query") {
        generator gen;

        auto e = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e, 42);
        std::get<1>(pools).emplace(e, 'x');

        auto& value = forge::algorithms::containers::pool_of<0, entity, int, char>(e, pools);
        static_assert(std::is_same_v<decltype(value), int&>);

        CHECK(value == 42);

        value = 100;

        CHECK(std::get<0>(pools).get(e) == 100);
    }

    TEST_CASE("pool_of returns const component for const query") {
        generator gen;

        auto e = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e, 42);
        std::get<1>(pools).emplace(e, 'x');

        auto& value = forge::algorithms::containers::pool_of<0, entity, const int, char>(e, pools);
        static_assert(std::is_same_v<decltype(value), const int&>);

        CHECK(value == 42);
    }

    TEST_CASE("pool_of preserves mutable and const queries independently") {
        generator gen;

        auto e = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e, 42);
        std::get<1>(pools).emplace(e, 'x');

        auto& i = forge::algorithms::containers::pool_of<0, entity, const int, char>(e, pools);
        auto& c = forge::algorithms::containers::pool_of<1, entity, const int, char>(e, pools);

        static_assert(std::is_same_v<decltype(i), const int&>);

        static_assert(std::is_same_v<decltype(c), char&>);

        CHECK(i == 42);
        CHECK(c == 'x');

        c = 'z';

        CHECK(std::get<1>(pools).get(e) == 'z');
    }

    TEST_CASE("pool_of accesses the correct entity") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 10);
        std::get<0>(pools).emplace(e1, 20);

        CHECK(forge::algorithms::containers::pool_of<0, entity, int>(e0, pools) == 10);
        CHECK(forge::algorithms::containers::pool_of<0, entity, int>(e1, pools) == 20);
    }
}
#include <doctest/doctest.h>

#include <forge/forge.hpp>
#include <iostream>

TEST_SUITE("view") {
    using entity = forge::entity;
    using generator = forge::generation::generator<entity>;

    using int_pool = forge::storage::sparse_set<int, entity>;
    using char_pool = forge::storage::sparse_set<char, entity>;
    using float_pool = forge::storage::sparse_set<float, entity>;

    using pools_type = std::tuple<
        int_pool,
        char_pool,
        float_pool>;

    TEST_CASE("view iterates entities having all included components") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();
        auto e2 = gen.make();

        pools_type pools;

        // e0 has int + char
        std::get<0>(pools).emplace(e0, 10);
        std::get<1>(pools).emplace(e0, 'a');

        // e1 has int + char
        std::get<0>(pools).emplace(e1, 20);
        std::get<1>(pools).emplace(e1, 'b');

        // e2 only has int
        std::get<0>(pools).emplace(e2, 30);

        using view_type =
            forge::basic_view_container<
                entity,
                pools_type,
                std::tuple<int, char>,
                std::tuple<>>;

        view_type view{pools};

        std::size_t count = 0;

        for (auto [i, c] : view) {
            CHECK((i == 10 || i == 20));
            CHECK((c == 'a' || c == 'b'));
            ++count;
        }

        CHECK(count == 2);
    }

    TEST_CASE("view uses smallest included pool as driving pool") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();
        auto e2 = gen.make();

        pools_type pools;

        // int: 3 entities
        std::get<0>(pools).emplace(e0, 10);
        std::get<0>(pools).emplace(e1, 20);
        std::get<0>(pools).emplace(e2, 30);

        // char: 2 entities
        std::get<1>(pools).emplace(e0, 'a');
        std::get<1>(pools).emplace(e1, 'b');

        // float: 1 entity
        std::get<2>(pools).emplace(e0, 1.0f);

        using view_type =
            forge::basic_view_container<
                entity,
                pools_type,
                std::tuple<int, char, float>,
                std::tuple<>>;

        view_type view{pools};

        std::size_t count = 0;

        for (auto [i, c, f] : view) {
            CHECK(i == 10);
            CHECK(c == 'a');
            CHECK(f == doctest::Approx(1.0f));
            ++count;
        }

        CHECK(count == 1);
    }

    TEST_CASE("view skips entities missing an included component") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();
        auto e2 = gen.make();

        pools_type pools;

        // All three have int.
        std::get<0>(pools).emplace(e0, 10);
        std::get<0>(pools).emplace(e1, 20);
        std::get<0>(pools).emplace(e2, 30);

        // Only e0 and e2 have char.
        std::get<1>(pools).emplace(e0, 'a');
        std::get<1>(pools).emplace(e2, 'c');

        using view_type =
            forge::basic_view_container<
                entity,
                pools_type,
                std::tuple<int, char>,
                std::tuple<>>;

        view_type view{pools};

        std::vector<int> values;

        for (auto [i, c] : view) {
            values.push_back(i);
        }

        CHECK(values.size() == 2);
        CHECK(values[0] == 10);
        CHECK(values[1] == 30);
    }

    TEST_CASE("view supports exclusion pools") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();
        auto e2 = gen.make();

        pools_type pools;

        // All entities have int.
        std::get<0>(pools).emplace(e0, 10);
        std::get<0>(pools).emplace(e1, 20);
        std::get<0>(pools).emplace(e2, 30);

        // e0 and e1 have char.
        std::get<1>(pools).emplace(e0, 'a');
        std::get<1>(pools).emplace(e1, 'b');

        // e1 has float and should therefore be excluded.
        std::get<2>(pools).emplace(e1, 2.0f);

        using view_type =
            forge::basic_view_container<
                entity,
                pools_type,
                std::tuple<int>,
                std::tuple<float>>;

        view_type view{pools};

        std::vector<int> values;

        for (auto [i] : view) {
            values.push_back(i);
        }

        CHECK(values.size() == 2);
        CHECK(values[0] == 10);
        CHECK(values[1] == 30);
    }

    TEST_CASE("view can include and exclude simultaneously") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();
        auto e2 = gen.make();

        pools_type pools;

        // Everyone has int.
        std::get<0>(pools).emplace(e0, 10);
        std::get<0>(pools).emplace(e1, 20);
        std::get<0>(pools).emplace(e2, 30);

        // e0 and e1 have char.
        std::get<1>(pools).emplace(e0, 'a');
        std::get<1>(pools).emplace(e1, 'b');

        // e1 should be excluded.
        std::get<2>(pools).emplace(e1, 2.0f);

        using view_type =
            forge::basic_view_container<
                entity,
                pools_type,
                std::tuple<int, char>,
                std::tuple<float>>;

        view_type view{pools};

        std::size_t count = 0;

        for (auto [i, c] : view) {
            CHECK(i == 10);
            CHECK(c == 'a');
            ++count;
        }

        CHECK(count == 1);
    }

    TEST_CASE("view returns mutable components") {
        generator gen;

        auto e0 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 10);
        std::get<1>(pools).emplace(e0, 'a');

        using view_type =
            forge::basic_view_container<
                entity,
                pools_type,
                std::tuple<int, char>,
                std::tuple<>>;

        view_type view{pools};

        for (auto [i, c] : view) {
            i = 42;
            c = 'z';
        }

        CHECK(std::get<0>(pools).get(e0) == 42);
        CHECK(std::get<1>(pools).get(e0) == 'z');
    }

    TEST_CASE("view returns components in requested order") {
        generator gen;

        auto e0 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 42);
        std::get<1>(pools).emplace(e0, 'x');
        std::get<2>(pools).emplace(e0, 3.14f);

        using view_type =
            forge::basic_view_container<
                entity,
                pools_type,
                std::tuple<float, int, char>,
                std::tuple<>>;

        view_type view{pools};

        for (auto [f, i, c] : view) {
            CHECK(f == doctest::Approx(3.14f));
            CHECK(i == 42);
            CHECK(c == 'x');
        }
    }

    TEST_CASE("empty included pool produces empty view") {
        generator gen;

        auto e0 = gen.make();

        pools_type pools;

        std::get<0>(pools).emplace(e0, 42);
        std::get<1>(pools).emplace(e0, 'x');

        // float pool is empty.

        using view_type =
            forge::basic_view_container<
                entity,
                pools_type,
                std::tuple<int, char, float>,
                std::tuple<>>;

        view_type view{pools};

        CHECK(view.begin() == view.end());
    }

    TEST_CASE("view handles multiple entities with different component combinations") {
        generator gen;

        auto e0 = gen.make();
        auto e1 = gen.make();
        auto e2 = gen.make();
        auto e3 = gen.make();

        pools_type pools;

        // e0: int + char
        std::get<0>(pools).emplace(e0, 10);
        std::get<1>(pools).emplace(e0, 'a');

        // e1: int only
        std::get<0>(pools).emplace(e1, 20);

        // e2: int + char
        std::get<0>(pools).emplace(e2, 30);
        std::get<1>(pools).emplace(e2, 'c');

        // e3: char only
        std::get<1>(pools).emplace(e3, 'd');

        using view_type =
            forge::basic_view_container<
                entity,
                pools_type,
                std::tuple<int, char>,
                std::tuple<>>;

        view_type view{pools};

        std::vector<int> values;

        for (auto [i, c] : view) {
            values.push_back(i);
        }

        CHECK(values.size() == 2);
        CHECK(values[0] == 10);
        CHECK(values[1] == 30);
    }

    TEST_CASE("world view only returns entities containing every component") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();
        auto e3 = world.make();

        // e0: int + char
        world.add_component<int>(e0, 10);
        world.add_component<char>(e0, 'a');

        // e1: int only
        world.add_component<int>(e1, 20);

        // e2: char only
        world.add_component<char>(e2, 'b');

        // e3: int + char
        world.add_component<int>(e3, 30);
        world.add_component<char>(e3, 'c');

        auto view = world.view<int, char>();

        std::vector<forge::entity> entities;

        for (auto e : view) {
            entities.push_back(e);
        }

        CHECK(entities.size() == 2);
        CHECK(entities[0] == e0);
        CHECK(entities[1] == e3);
    }

    TEST_CASE("world view uses smallest component pool as driving pool") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();

        // int: 3 entities
        world.add_component<int>(e0, 10);
        world.add_component<int>(e1, 20);
        world.add_component<int>(e2, 30);

        // char: 2 entities
        world.add_component<char>(e0, 'a');
        world.add_component<char>(e1, 'b');

        auto view = world.view<int, char>();

        std::vector<forge::entity> entities;

        for (auto e : view) {
            entities.push_back(e);
        }

        CHECK(entities.size() == 2);
        CHECK(entities[0] == e0);
        CHECK(entities[1] == e1);
    }

    TEST_CASE("world view is empty when an included pool is empty") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();

        world.add_component<int>(e0, 10);
        world.add_component<int>(e1, 20);

        world.add_component<char>(e0, 'a');
        world.add_component<char>(e1, 'b');

        // float is completely empty.
        auto view = world.view<int, char, float>();

        CHECK(view.begin() == view.end());

        std::size_t count = 0;

        for (auto e : view) {
            (void)e;
            ++count;
        }

        CHECK(count == 0);
    }

    TEST_CASE("world view finds single matching entity") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();

        world.add_component<int>(e0, 10);
        world.add_component<int>(e1, 20);
        world.add_component<int>(e2, 30);

        world.add_component<char>(e1, 'x');

        auto view = world.view<int, char>();

        auto it = view.begin();

        REQUIRE(it != view.end());
        CHECK(*it == e1);

        ++it;

        CHECK(it == view.end());
    }

    TEST_CASE("world view dereferences to entity") {
        forge::registry<int, char> world;

        auto e0 = world.make();

        world.add_component<int>(e0, 42);
        world.add_component<char>(e0, 'x');

        auto view = world.view<int, char>();

        auto it = view.begin();

        REQUIRE(it != view.end());

        auto e = *it;

        CHECK(e == e0);
    }

    TEST_CASE("world view each provides component references") {
        forge::registry<int, char> world;

        auto e0 = world.make();

        world.add_component<int>(e0, 42);
        world.add_component<char>(e0, 'x');

        auto view = world.view<int, char>();

        view.each([](int& i, char& c) {
            CHECK(i == 42);
            CHECK(c == 'x');
        });
    }
}
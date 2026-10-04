#include <forge/forge.hpp>
#include <string>
#include <tuple>

#include "doctest.h"

using world = forge::registry<int, float, std::string>;
using entity = forge::entity;

TEST_CASE("registry starts empty") {
    world registry;

    const auto e = registry.make();

    CHECK(registry.is_alive(e));
    CHECK_FALSE(registry.has_component<int>(e));
    CHECK_FALSE(registry.has_component<float>(e));
    CHECK_FALSE(registry.has_component<std::string>(e));
}

TEST_CASE("registry can create multiple entities") {
    world registry;

    const auto e0 = registry.make();
    const auto e1 = registry.make();
    const auto e2 = registry.make();

    CHECK(registry.is_alive(e0));
    CHECK(registry.is_alive(e1));
    CHECK(registry.is_alive(e2));

    CHECK(e0 != e1);
    CHECK(e0 != e2);
    CHECK(e1 != e2);
}

TEST_CASE("registry can add a component") {
    world registry;

    const auto e = registry.make();

    auto& value = registry.add_component<int>(e, 42);

    CHECK(value == 42);
    CHECK(registry.has_component<int>(e));
    CHECK(registry.get_component<int>(e) == 42);
}

TEST_CASE("registry can add different component types") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 42);
    registry.add_component<float>(e, 3.14f);
    registry.add_component<std::string>(e, "hello");

    CHECK(registry.has_component<int>(e));
    CHECK(registry.has_component<float>(e));
    CHECK(registry.has_component<std::string>(e));

    CHECK(registry.get_component<int>(e) == 42);
    CHECK(registry.get_component<float>(e) == doctest::Approx(3.14f));
    CHECK(registry.get_component<std::string>(e) == "hello");
}

TEST_CASE("registry can modify a component through get_component") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 10);

    registry.get_component<int>(e) = 50;

    CHECK(registry.get_component<int>(e) == 50);
}

TEST_CASE("registry can retrieve multiple components") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 10);
    registry.add_component<float>(e, 20.0f);

    auto [i, f] = registry.get_component<int, float>(e);

    CHECK(i == 10);
    CHECK(f == doctest::Approx(20.0f));
}

TEST_CASE("registry try_get returns component when present") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 42);

    auto* value = registry.try_get<int>(e);

    REQUIRE(value != nullptr);
    CHECK(*value == 42);
}

TEST_CASE("registry try_get returns null when component is absent") {
    world registry;

    const auto e = registry.make();

    CHECK(registry.try_get<int>(e) == nullptr);
}

TEST_CASE("registry try_get returns null for dead entity") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 42);
    registry.destroy(e);

    CHECK(registry.try_get<int>(e) == nullptr);
}

TEST_CASE("registry can remove a component") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 42);

    CHECK(registry.has_component<int>(e));

    CHECK(registry.remove_component<int>(e));

    CHECK_FALSE(registry.has_component<int>(e));
}

TEST_CASE("removing a missing component returns false") {
    world registry;

    const auto e = registry.make();

    CHECK_FALSE(registry.remove_component<int>(e));
}

TEST_CASE("removing one component does not affect other components") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 10);
    registry.add_component<float>(e, 20.0f);

    CHECK(registry.remove_component<int>(e));

    CHECK_FALSE(registry.has_component<int>(e));
    CHECK(registry.has_component<float>(e));
    CHECK(registry.get_component<float>(e) == doctest::Approx(20.0f));
}

TEST_CASE("registry can remove multiple components") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 10);
    registry.add_component<float>(e, 20.0f);
    registry.add_component<std::string>(e, "hello");

    CHECK(registry.remove_component<int, float>(e));

    CHECK_FALSE(registry.has_component<int>(e));
    CHECK_FALSE(registry.has_component<float>(e));
    CHECK(registry.has_component<std::string>(e));
}

TEST_CASE("removing multiple components reports false if one is missing") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 10);

    CHECK_FALSE(registry.remove_component<int, float>(e));

    CHECK_FALSE(registry.has_component<int>(e));
}

TEST_CASE("registry can replace a component") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 10);

    auto& value = registry.replace_component<int>(e, 50);

    CHECK(value == 50);
    CHECK(registry.has_component<int>(e));
    CHECK(registry.get_component<int>(e) == 50);
}

TEST_CASE("registry can replace multiple components") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 10);
    registry.add_component<float>(e, 20.0f);

    auto [i, f] = registry.replace_component<int, float>(e, 100, 200.0f);

    CHECK(i == 100);
    CHECK(f == doctest::Approx(200.0f));
}

TEST_CASE("add_or_replace adds missing component") {
    world registry;

    const auto e = registry.make();

    auto& value = registry.add_or_replace_component<int>(e, 42);

    CHECK(value == 42);
    CHECK(registry.has_component<int>(e));
}

TEST_CASE("add_or_replace replaces existing component") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 42);

    auto& value = registry.add_or_replace_component<int>(e, 100);

    CHECK(value == 100);
    CHECK(registry.has_component<int>(e));
    CHECK(registry.get_component<int>(e) == 100);
}

TEST_CASE("add_or_replace supports multiple components") {
    world registry;

    const auto e = registry.make();

    auto [i, f] =
        registry.add_or_replace_component<int, float>(e, 10, 20.0f);

    CHECK(i == 10);
    CHECK(f == doctest::Approx(20.0f));

    auto [i2, f2] =
        registry.add_or_replace_component<int, float>(e, 30, 40.0f);

    CHECK(i2 == 30);
    CHECK(f2 == doctest::Approx(40.0f));
}

TEST_CASE("registry destroy removes all components") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 10);
    registry.add_component<float>(e, 20.0f);
    registry.add_component<std::string>(e, "hello");

    REQUIRE(registry.is_alive(e));

    registry.destroy(e);

    CHECK_FALSE(registry.is_alive(e));
    CHECK_FALSE(registry.has_component<int>(e));
    CHECK_FALSE(registry.has_component<float>(e));
    CHECK_FALSE(registry.has_component<std::string>(e));
}

TEST_CASE("destroying an invalid entity does nothing") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 42);
    registry.destroy(e);

    CHECK_FALSE(registry.is_alive(e));

    registry.destroy(e);

    CHECK_FALSE(registry.is_alive(e));
}

TEST_CASE("destroying one entity does not affect another") {
    world registry;

    const auto e0 = registry.make();
    const auto e1 = registry.make();

    registry.add_component<int>(e0, 10);
    registry.add_component<int>(e1, 20);

    registry.destroy(e0);

    CHECK_FALSE(registry.is_alive(e0));
    CHECK(registry.is_alive(e1));

    CHECK_FALSE(registry.has_component<int>(e0));
    CHECK(registry.has_component<int>(e1));
    CHECK(registry.get_component<int>(e1) == 20);
}

TEST_CASE("entity recycling invalidates the old generation") {
    world registry;

    const auto e0 = registry.make();

    registry.add_component<int>(e0, 42);
    registry.destroy(e0);

    const auto e1 = registry.make();

    CHECK(e0 != e1);
    CHECK_FALSE(registry.is_alive(e0));
    CHECK(registry.is_alive(e1));

    CHECK_FALSE(registry.has_component<int>(e0));
    CHECK_FALSE(registry.has_component<int>(e1));
}

TEST_CASE("recycled entity can receive components") {
    world registry;

    const auto old_entity = registry.make();

    registry.add_component<int>(old_entity, 42);
    registry.destroy(old_entity);

    const auto new_entity = registry.make();

    registry.add_component<int>(new_entity, 100);

    CHECK_FALSE(registry.has_component<int>(old_entity));
    CHECK(registry.has_component<int>(new_entity));
    CHECK(registry.get_component<int>(new_entity) == 100);
}

TEST_CASE("construct signal fires when component is added") {
    world registry;

    const auto e = registry.make();

    bool called = false;
    entity received_entity{};
    int received_value{};

    registry.on_construct<int>().connect(
        [&](entity received, int& value) {
            called = true;
            received_entity = received;
            received_value = value;
        });

    registry.add_component<int>(e, 42);

    CHECK(called);
    CHECK(received_entity == e);
    CHECK(received_value == 42);
}

TEST_CASE("update signal fires when component is replaced") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 10);

    bool called = false;
    entity received_entity{};
    int received_value{};

    registry.on_update<int>().connect(
        [&](entity received, int& value) {
            called = true;
            received_entity = received;
            received_value = value;
        });

    registry.replace_component<int>(e, 50);

    CHECK(called);
    CHECK(received_entity == e);
    CHECK(received_value == 50);
}

TEST_CASE("destroy signal fires when component is explicitly removed") {
    world registry;

    const auto e = registry.make();

    registry.add_component<int>(e, 42);

    bool called = false;
    entity received_entity{};
    int received_value{};

    registry.on_destroy<int>().connect(
        [&](entity received, int& value) {
            called = true;
            received_entity = received;
            received_value = value;
        });

    registry.remove_component<int>(e);

    CHECK(called);
    CHECK(received_entity == e);
    CHECK(received_value == 42);
}

TEST_CASE("multiple callbacks can connect to the same signal") {
    world registry;

    const auto e = registry.make();

    int first = 0;
    int second = 0;

    registry.on_construct<int>().connect(
        [&](entity, int&) {
            ++first;
        });

    registry.on_construct<int>().connect(
        [&](entity, int&) {
            ++second;
        });

    registry.add_component<int>(e, 42);

    CHECK(first == 1);
    CHECK(second == 1);
}

TEST_CASE("const get_component returns const component") {
    world registry;
    const auto e = registry.make();
    registry.add_component<int>(e, 42);
    const world& const_registry = registry;
    const int& value = const_registry.get_component<const int>(e);
    CHECK(value == 42);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(value)>>);
}
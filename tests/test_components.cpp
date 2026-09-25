#include <forge/forge.hpp>

#include "doctest.h"

TEST_SUITE("components") {
    // ============================================================
    // has_component
    // ============================================================

    TEST_CASE("new entity has no components") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        CHECK(!world.has_component<int>(e));
        CHECK(!world.has_component<char>(e));
        CHECK(!world.has_component<float>(e));
    }

    TEST_CASE("has_component returns true after adding component") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);

        CHECK(world.has_component<int>(e));
        CHECK(!world.has_component<char>(e));
        CHECK(!world.has_component<float>(e));
    }

    TEST_CASE("has_component works independently for each component") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);
        world.add_component<float>(e, 3.14f);

        CHECK(world.has_component<int>(e));
        CHECK(!world.has_component<char>(e));
        CHECK(world.has_component<float>(e));
    }

    TEST_CASE("has_component supports logical AND") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);
        world.add_component<char>(e, 'a');

        CHECK(world.has_component<int, char>(e));
        CHECK(!world.has_component<int, float>(e));
        CHECK(!world.has_component<char, float>(e));
    }

    TEST_CASE("has_component supports logical OR") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);

        CHECK(world.has_component<std::logical_or, int, float>(e));
        CHECK(world.has_component<std::logical_or, char, int>(e));
        CHECK(!world.has_component<std::logical_or, char, float>(e));
    }

    // ============================================================
    // add_component
    // ============================================================

    TEST_CASE("add_component returns reference to inserted component") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        auto& component = world.add_component<int>(e, 42);

        CHECK(component == 42);
        CHECK(&component == &world.get_component<int>(e));
    }

    TEST_CASE("add_component constructs component with arguments") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        auto [intC, floatC] = world.add_component<int, float>(e, 10, 3.5f);
        CHECK(intC == 10);
        CHECK(floatC == 3.5);
    }

    TEST_CASE("different entities can have same component type") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();

        world.add_component<int>(e0, 10);
        world.add_component<int>(e1, 20);

        CHECK(world.get_component<int>(e0) == 10);
        CHECK(world.get_component<int>(e1) == 20);
    }

    TEST_CASE("one entity can have multiple component types") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);
        world.add_component<char>(e, 'x');
        world.add_component<float>(e, 3.14f);

        CHECK(world.has_component<int>(e));
        CHECK(world.has_component<char>(e));
        CHECK(world.has_component<float>(e));

        CHECK(world.get_component<int>(e) == 42);
        CHECK(world.get_component<char>(e) == 'x');
        CHECK(world.get_component<float>(e) == doctest::Approx(3.14f));
    }
    // ============================================================
    // Multi-component get
    // ============================================================

    TEST_CASE("get_component can retrieve multiple components") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);
        world.add_component<char>(e, 'x');

        auto [i, c] = world.get_component<int, char>(e);

        CHECK(i == 42);
        CHECK(c == 'x');
    }

    TEST_CASE("multi-component get returns references") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);
        world.add_component<char>(e, 'x');

        auto [i, c] = world.get_component<int, char>(e);

        i = 100;
        c = 'z';

        CHECK(world.get_component<int>(e) == 100);
        CHECK(world.get_component<char>(e) == 'z');
    }

    TEST_CASE("multi-component get preserves const components") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);
        world.add_component<char>(e, 'x');

        auto [i, c] = world.get_component<const int, char>(e);

        static_assert(std::is_const_v<std::remove_reference_t<decltype(i)>>);
        static_assert(!std::is_const_v<std::remove_reference_t<decltype(c)>>);

        CHECK(i == 42);
        CHECK(c == 'x');
    }

    // ============================================================
    // remove_component
    // ============================================================

    TEST_CASE("remove_component removes existing component") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);

        CHECK(world.has_component<int>(e));

        bool removed = world.remove_component<int>(e);

        CHECK(removed);
        CHECK(!world.has_component<int>(e));
    }

    TEST_CASE("remove_component returns false when component is absent") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        CHECK(!world.has_component<int>(e));

        bool removed = world.remove_component<int>(e);

        CHECK(!removed);
    }

    TEST_CASE("removing one component does not remove others") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);
        world.add_component<char>(e, 'x');
        world.add_component<float>(e, 3.14f);

        world.remove_component<char>(e);

        CHECK(world.has_component<int>(e));
        CHECK(!world.has_component<char>(e));
        CHECK(world.has_component<float>(e));

        CHECK(world.get_component<int>(e) == 42);
        CHECK(world.get_component<float>(e) == doctest::Approx(3.14f));
    }

    TEST_CASE("removing component from one entity does not affect another") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();

        world.add_component<int>(e0, 10);
        world.add_component<int>(e1, 20);

        world.remove_component<int>(e0);

        CHECK(!world.has_component<int>(e0));
        CHECK(world.has_component<int>(e1));

        CHECK(world.get_component<int>(e1) == 20);
    }

    TEST_CASE("remove multiple components") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);
        world.add_component<char>(e, 'x');
        world.add_component<float>(e, 3.14f);

        bool removed = world.remove_component<int, char>(e);

        CHECK(removed);

        CHECK(!world.has_component<int>(e));
        CHECK(!world.has_component<char>(e));
        CHECK(world.has_component<float>(e));
    }

    // ============================================================
    // replace_component
    // ============================================================

    TEST_CASE("replace_component changes existing component") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 10);

        auto& component = world.replace_component<int>(e, 20);

        CHECK(component == 20);
        CHECK(world.get_component<int>(e) == 20);
    }

    TEST_CASE("replace_component preserves entity component membership") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 10);

        world.replace_component<int>(e, 20);

        CHECK(world.has_component<int>(e));
    }

    TEST_CASE("replace_component returns reference to replaced component") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 10);

        auto& component = world.replace_component<int>(e, 20);

        component = 30;

        CHECK(world.get_component<int>(e) == 30);
    }

    TEST_CASE("replace multiple components") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 10);
        world.add_component<char>(e, 'a');

        auto [i, c] =
            world.replace_component<int, char>(e, 20, 'b');

        CHECK(i == 20);
        CHECK(c == 'b');

        CHECK(world.get_component<int>(e) == 20);
        CHECK(world.get_component<char>(e) == 'b');
    }

    // ============================================================
    // add_or_replace_component
    // ============================================================

    TEST_CASE("add_or_replace adds missing component") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        CHECK(!world.has_component<int>(e));

        auto& component = world.add_or_replace_component<int>(e, 42);

        CHECK(world.has_component<int>(e));
        CHECK(component == 42);
    }

    TEST_CASE("add_or_replace replaces existing component") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 10);

        auto& component =
            world.add_or_replace_component<int>(e, 42);

        CHECK(world.has_component<int>(e));
        CHECK(component == 42);
        CHECK(world.get_component<int>(e) == 42);
    }

    TEST_CASE("add_or_replace does not affect other components") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 10);
        world.add_component<char>(e, 'a');

        world.add_or_replace_component<int>(e, 20);

        CHECK(world.get_component<int>(e) == 20);
        CHECK(world.get_component<char>(e) == 'a');
    }

    TEST_CASE("add_or_replace multiple components") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        auto [i, c] =
            world.add_or_replace_component<int, char>(e, 42, 'x');

        CHECK(i == 42);
        CHECK(c == 'x');

        CHECK(world.has_component<int>(e));
        CHECK(world.has_component<char>(e));

        world.add_or_replace_component<int, char>(e, 100, 'z');

        CHECK(world.get_component<int>(e) == 100);
        CHECK(world.get_component<char>(e) == 'z');
    }
    // ============================================================
    // Entity destruction + components
    // ============================================================

    TEST_CASE("destroy removes all components") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 42);
        world.add_component<char>(e, 'x');
        world.add_component<float>(e, 3.14f);

        CHECK(world.has_component<int>(e));
        CHECK(world.has_component<char>(e));
        CHECK(world.has_component<float>(e));

        world.destroy(e);

        CHECK(!world.is_alive(e));
        CHECK(!world.has_component<int>(e));
        CHECK(!world.has_component<char>(e));
        CHECK(!world.has_component<float>(e));
    }

    TEST_CASE("destroying one entity does not remove another entity's components") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();

        world.add_component<int>(e0, 10);
        world.add_component<int>(e1, 20);

        world.destroy(e0);

        CHECK(!world.has_component<int>(e0));
        CHECK(world.has_component<int>(e1));

        CHECK(world.get_component<int>(e1) == 20);
    }

    TEST_CASE("recycled entity starts without old components") {
        forge::registry<int, char, float> world;

        auto old = world.make();

        world.add_component<int>(old, 42);
        world.add_component<char>(old, 'x');

        world.destroy(old);

        auto current = world.make();

        CHECK(forge::to_id(old) == forge::to_id(current));
        CHECK(forge::to_version(current) == 1);

        CHECK(!world.has_component<int>(current));
        CHECK(!world.has_component<char>(current));
        CHECK(!world.has_component<float>(current));
    }

    TEST_CASE("recycled entity can receive new components") {
        forge::registry<int, char, float> world;

        auto old = world.make();

        world.add_component<int>(old, 42);
        world.destroy(old);

        auto current = world.make();

        world.add_component<int>(current, 100);

        CHECK(world.has_component<int>(current));
        CHECK(world.get_component<int>(current) == 100);
    }

    TEST_CASE("old entity cannot retain component membership after recycling") {
        forge::registry<int, char, float> world;

        auto old = world.make();

        world.add_component<int>(old, 42);

        world.destroy(old);

        auto current = world.make();

        CHECK(!world.has_component<int>(old));
        CHECK(!world.has_component<int>(current));
    }

    TEST_CASE("component storage remains correct across entity recycling") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();

        world.add_component<int>(e0, 10);
        world.add_component<int>(e1, 20);

        world.destroy(e0);

        auto e2 = world.make();

        world.add_component<int>(e2, 30);

        CHECK(world.get_component<int>(e1) == 20);
        CHECK(world.get_component<int>(e2) == 30);

        CHECK(!world.has_component<int>(e0));
    }

    // ============================================================
    // Component independence
    // ============================================================

    TEST_CASE("component types have independent storage") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        world.add_component<int>(e, 10);
        world.add_component<char>(e, 'a');
        world.add_component<float>(e, 1.5f);

        world.get_component<int>(e) = 20;
        world.get_component<char>(e) = 'b';
        world.get_component<float>(e) = 2.5f;

        CHECK(world.get_component<int>(e) == 20);
        CHECK(world.get_component<char>(e) == 'b');
        CHECK(world.get_component<float>(e) == doctest::Approx(2.5f));
    }

    // ============================================================
    // Multiple entities + multiple components
    // ============================================================

    TEST_CASE("components remain associated with correct entities") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();

        world.add_component<int>(e0, 10);
        world.add_component<char>(e0, 'a');

        world.add_component<int>(e1, 20);
        world.add_component<float>(e1, 2.0f);

        world.add_component<char>(e2, 'z');
        world.add_component<float>(e2, 3.0f);

        CHECK(world.get_component<int>(e0) == 10);
        CHECK(world.get_component<char>(e0) == 'a');

        CHECK(world.get_component<int>(e1) == 20);
        CHECK(world.get_component<float>(e1) == doctest::Approx(2.0f));

        CHECK(world.get_component<char>(e2) == 'z');
        CHECK(world.get_component<float>(e2) == doctest::Approx(3.0f));
    }

    TEST_CASE("removing components from multiple entities is independent") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();

        world.add_component<int>(e0, 10);
        world.add_component<int>(e1, 20);
        world.add_component<int>(e2, 30);

        world.remove_component<int>(e1);

        CHECK(world.has_component<int>(e0));
        CHECK(!world.has_component<int>(e1));
        CHECK(world.has_component<int>(e2));

        CHECK(world.get_component<int>(e0) == 10);
        CHECK(world.get_component<int>(e2) == 30);
    }

    // ============================================================
    // Stress tests
    // ============================================================

    TEST_CASE("many entities can have components") {
        forge::registry<int, char, float> world;

        constexpr int count = 1000;

        std::vector<forge::entity> entities;

        for (int i = 0; i < count; ++i) {
            auto e = world.make();

            world.add_component<int>(e, i);

            entities.push_back(e);
        }

        for (int i = 0; i < count; ++i) {
            CHECK(world.has_component<int>(entities[i]));
            CHECK(world.get_component<int>(entities[i]) == i);
        }
    }

    TEST_CASE("many entities can add and remove components") {
        forge::registry<int, char, float> world;

        constexpr int count = 1000;

        std::vector<forge::entity> entities;

        for (int i = 0; i < count; ++i) {
            auto e = world.make();

            world.add_component<int>(e, i);
            entities.push_back(e);
        }

        for (auto e : entities) {
            CHECK(world.has_component<int>(e));
            CHECK(world.remove_component<int>(e));
            CHECK(!world.has_component<int>(e));
        }
    }
}
// ============================================================
// has_component
// ============================================================

TEST_CASE("new entity has no components") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    CHECK(!world.has_component<int>(e));
    CHECK(!world.has_component<char>(e));
    CHECK(!world.has_component<float>(e));
}

TEST_CASE("has_component returns true after adding component") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);

    CHECK(world.has_component<int>(e));
    CHECK(!world.has_component<char>(e));
    CHECK(!world.has_component<float>(e));
}

TEST_CASE("has_component works independently for each component") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);
    world.add_component<float>(e, 3.14f);

    CHECK(world.has_component<int>(e));
    CHECK(!world.has_component<char>(e));
    CHECK(world.has_component<float>(e));
}

TEST_CASE("has_component supports logical AND") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);
    world.add_component<char>(e, 'a');

    CHECK(world.has_component<int, char>(e));
    CHECK(!world.has_component<int, float>(e));
    CHECK(!world.has_component<char, float>(e));
}

TEST_CASE("has_component supports logical OR") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);

    CHECK(world.has_component<std::logical_or, int, float>(e));
    CHECK(world.has_component<std::logical_or, char, int>(e));
    CHECK(!world.has_component<std::logical_or, char, float>(e));
}

// ============================================================
// add_component
// ============================================================

TEST_CASE("add_component returns reference to inserted component") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    auto& component = world.add_component<int>(e, 42);

    CHECK(component == 42);
    CHECK(&component == &world.get_component<int>(e));
}

TEST_CASE("add_component constructs component with arguments") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    auto [intC, floatC] = world.add_component<int, float>(e, 10, 3.5f);
    CHECK(intC == 10);
    CHECK(floatC == 3.5);
}

TEST_CASE("different entities can have same component type") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();

    world.add_component<int>(e0, 10);
    world.add_component<int>(e1, 20);

    CHECK(world.get_component<int>(e0) == 10);
    CHECK(world.get_component<int>(e1) == 20);
}

TEST_CASE("one entity can have multiple component types") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);
    world.add_component<char>(e, 'x');
    world.add_component<float>(e, 3.14f);

    CHECK(world.has_component<int>(e));
    CHECK(world.has_component<char>(e));
    CHECK(world.has_component<float>(e));

    CHECK(world.get_component<int>(e) == 42);
    CHECK(world.get_component<char>(e) == 'x');
    CHECK(world.get_component<float>(e) == doctest::Approx(3.14f));
}
// ============================================================
// Multi-component get
// ============================================================

TEST_CASE("get_component can retrieve multiple components") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);
    world.add_component<char>(e, 'x');

    auto [i, c] = world.get_component<int, char>(e);

    CHECK(i == 42);
    CHECK(c == 'x');
}

TEST_CASE("multi-component get returns references") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);
    world.add_component<char>(e, 'x');

    auto [i, c] = world.get_component<int, char>(e);

    i = 100;
    c = 'z';

    CHECK(world.get_component<int>(e) == 100);
    CHECK(world.get_component<char>(e) == 'z');
}

TEST_CASE("multi-component get preserves const components") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);
    world.add_component<char>(e, 'x');

    auto [i, c] = world.get_component<const int, char>(e);

    static_assert(std::is_const_v<std::remove_reference_t<decltype(i)>>);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(c)>>);

    CHECK(i == 42);
    CHECK(c == 'x');
}

// ============================================================
// remove_component
// ============================================================

TEST_CASE("remove_component removes existing component") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);

    CHECK(world.has_component<int>(e));

    bool removed = world.remove_component<int>(e);

    CHECK(removed);
    CHECK(!world.has_component<int>(e));
}

TEST_CASE("remove_component returns false when component is absent") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    CHECK(!world.has_component<int>(e));

    bool removed = world.remove_component<int>(e);

    CHECK(!removed);
}

TEST_CASE("removing one component does not remove others") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);
    world.add_component<char>(e, 'x');
    world.add_component<float>(e, 3.14f);

    world.remove_component<char>(e);

    CHECK(world.has_component<int>(e));
    CHECK(!world.has_component<char>(e));
    CHECK(world.has_component<float>(e));

    CHECK(world.get_component<int>(e) == 42);
    CHECK(world.get_component<float>(e) == doctest::Approx(3.14f));
}

TEST_CASE("removing component from one entity does not affect another") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();

    world.add_component<int>(e0, 10);
    world.add_component<int>(e1, 20);

    world.remove_component<int>(e0);

    CHECK(!world.has_component<int>(e0));
    CHECK(world.has_component<int>(e1));

    CHECK(world.get_component<int>(e1) == 20);
}

TEST_CASE("remove multiple components") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);
    world.add_component<char>(e, 'x');
    world.add_component<float>(e, 3.14f);

    bool removed = world.remove_component<int, char>(e);

    CHECK(removed);

    CHECK(!world.has_component<int>(e));
    CHECK(!world.has_component<char>(e));
    CHECK(world.has_component<float>(e));
}

// ============================================================
// replace_component
// ============================================================

TEST_CASE("replace_component changes existing component") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 10);

    auto& component = world.replace_component<int>(e, 20);

    CHECK(component == 20);
    CHECK(world.get_component<int>(e) == 20);
}

TEST_CASE("replace_component preserves entity component membership") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 10);

    world.replace_component<int>(e, 20);

    CHECK(world.has_component<int>(e));
}

TEST_CASE("replace_component returns reference to replaced component") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 10);

    auto& component = world.replace_component<int>(e, 20);

    component = 30;

    CHECK(world.get_component<int>(e) == 30);
}

TEST_CASE("replace multiple components") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 10);
    world.add_component<char>(e, 'a');

    auto [i, c] =
        world.replace_component<int, char>(e, 20, 'b');

    CHECK(i == 20);
    CHECK(c == 'b');

    CHECK(world.get_component<int>(e) == 20);
    CHECK(world.get_component<char>(e) == 'b');
}

// ============================================================
// add_or_replace_component
// ============================================================

TEST_CASE("add_or_replace adds missing component") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    CHECK(!world.has_component<int>(e));

    auto& component = world.add_or_replace_component<int>(e, 42);

    CHECK(world.has_component<int>(e));
    CHECK(component == 42);
}

TEST_CASE("add_or_replace replaces existing component") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 10);

    auto& component =
        world.add_or_replace_component<int>(e, 42);

    CHECK(world.has_component<int>(e));
    CHECK(component == 42);
    CHECK(world.get_component<int>(e) == 42);
}

TEST_CASE("add_or_replace does not affect other components") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 10);
    world.add_component<char>(e, 'a');

    world.add_or_replace_component<int>(e, 20);

    CHECK(world.get_component<int>(e) == 20);
    CHECK(world.get_component<char>(e) == 'a');
}

TEST_CASE("add_or_replace multiple components") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    auto [i, c] =
        world.add_or_replace_component<int, char>(e, 42, 'x');

    CHECK(i == 42);
    CHECK(c == 'x');

    CHECK(world.has_component<int>(e));
    CHECK(world.has_component<char>(e));

    world.add_or_replace_component<int, char>(e, 100, 'z');

    CHECK(world.get_component<int>(e) == 100);
    CHECK(world.get_component<char>(e) == 'z');
}
// ============================================================
// Entity destruction + components
// ============================================================

TEST_CASE("destroy removes all components") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 42);
    world.add_component<char>(e, 'x');
    world.add_component<float>(e, 3.14f);

    CHECK(world.has_component<int>(e));
    CHECK(world.has_component<char>(e));
    CHECK(world.has_component<float>(e));

    world.destroy(e);

    CHECK(!world.is_alive(e));
    CHECK(!world.has_component<int>(e));
    CHECK(!world.has_component<char>(e));
    CHECK(!world.has_component<float>(e));
}

TEST_CASE("destroying one entity does not remove another entity's components") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();

    world.add_component<int>(e0, 10);
    world.add_component<int>(e1, 20);

    world.destroy(e0);

    CHECK(!world.has_component<int>(e0));
    CHECK(world.has_component<int>(e1));

    CHECK(world.get_component<int>(e1) == 20);
}

TEST_CASE("recycled entity starts without old components") {
    forge::registry<int, char, float> world;

    auto old = world.make();

    world.add_component<int>(old, 42);
    world.add_component<char>(old, 'x');

    world.destroy(old);

    auto current = world.make();

    CHECK(forge::to_id(old) == forge::to_id(current));
    CHECK(forge::to_version(current) == 1);

    CHECK(!world.has_component<int>(current));
    CHECK(!world.has_component<char>(current));
    CHECK(!world.has_component<float>(current));
}

TEST_CASE("recycled entity can receive new components") {
    forge::registry<int, char, float> world;

    auto old = world.make();

    world.add_component<int>(old, 42);
    world.destroy(old);

    auto current = world.make();

    world.add_component<int>(current, 100);

    CHECK(world.has_component<int>(current));
    CHECK(world.get_component<int>(current) == 100);
}

TEST_CASE("old entity cannot retain component membership after recycling") {
    forge::registry<int, char, float> world;

    auto old = world.make();

    world.add_component<int>(old, 42);

    world.destroy(old);

    auto current = world.make();

    CHECK(!world.has_component<int>(old));
    CHECK(!world.has_component<int>(current));
}

TEST_CASE("component storage remains correct across entity recycling") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();

    world.add_component<int>(e0, 10);
    world.add_component<int>(e1, 20);

    world.destroy(e0);

    auto e2 = world.make();

    world.add_component<int>(e2, 30);

    CHECK(world.get_component<int>(e1) == 20);
    CHECK(world.get_component<int>(e2) == 30);

    CHECK(!world.has_component<int>(e0));
}

// ============================================================
// Component independence
// ============================================================

TEST_CASE("component types have independent storage") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    world.add_component<int>(e, 10);
    world.add_component<char>(e, 'a');
    world.add_component<float>(e, 1.5f);

    world.get_component<int>(e) = 20;
    world.get_component<char>(e) = 'b';
    world.get_component<float>(e) = 2.5f;

    CHECK(world.get_component<int>(e) == 20);
    CHECK(world.get_component<char>(e) == 'b');
    CHECK(world.get_component<float>(e) == doctest::Approx(2.5f));
}

// ============================================================
// Multiple entities + multiple components
// ============================================================

TEST_CASE("components remain associated with correct entities") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();
    auto e2 = world.make();

    world.add_component<int>(e0, 10);
    world.add_component<char>(e0, 'a');

    world.add_component<int>(e1, 20);
    world.add_component<float>(e1, 2.0f);

    world.add_component<char>(e2, 'z');
    world.add_component<float>(e2, 3.0f);

    CHECK(world.get_component<int>(e0) == 10);
    CHECK(world.get_component<char>(e0) == 'a');

    CHECK(world.get_component<int>(e1) == 20);
    CHECK(world.get_component<float>(e1) == doctest::Approx(2.0f));

    CHECK(world.get_component<char>(e2) == 'z');
    CHECK(world.get_component<float>(e2) == doctest::Approx(3.0f));
}

TEST_CASE("removing components from multiple entities is independent") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();
    auto e2 = world.make();

    world.add_component<int>(e0, 10);
    world.add_component<int>(e1, 20);
    world.add_component<int>(e2, 30);

    world.remove_component<int>(e1);

    CHECK(world.has_component<int>(e0));
    CHECK(!world.has_component<int>(e1));
    CHECK(world.has_component<int>(e2));

    CHECK(world.get_component<int>(e0) == 10);
    CHECK(world.get_component<int>(e2) == 30);
}

// ============================================================
// Stress tests
// ============================================================

TEST_CASE("many entities can have components") {
    forge::registry<int, char, float> world;

    constexpr int count = 1000;

    std::vector<forge::entity> entities;

    for (int i = 0; i < count; ++i) {
        auto e = world.make();

        world.add_component<int>(e, i);

        entities.push_back(e);
    }

    for (int i = 0; i < count; ++i) {
        CHECK(world.has_component<int>(entities[i]));
        CHECK(world.get_component<int>(entities[i]) == i);
    }
}

TEST_CASE("many entities can add and remove components") {
    forge::registry<int, char, float> world;

    constexpr int count = 1000;

    std::vector<forge::entity> entities;

    for (int i = 0; i < count; ++i) {
        auto e = world.make();

        world.add_component<int>(e, i);
        entities.push_back(e);
    }

    for (auto e : entities) {
        CHECK(world.has_component<int>(e));
        CHECK(world.remove_component<int>(e));
        CHECK(!world.has_component<int>(e));
    }
}
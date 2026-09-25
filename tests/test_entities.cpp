#include <forge/forge.hpp>

#include "doctest.h"

TEST_SUITE("entities") {
    TEST_CASE("world creates/destroys/reuses entity") {
        forge::registry<int, char, float> world;
        auto e = world.make();
        CHECK(forge::to_id(e) == 0);
        world.destroy(e);
        CHECK(!world.is_alive(e));
        auto e2 = world.make();
        CHECK(forge::to_id(e2) == 0);
        CHECK(forge::to_version(e2) == 1);
    }

    TEST_CASE("world creates multiple entities") {
        forge::registry<int, char, float> world;
        for (int i = 0; i < 10; i++) {
            auto e = world.make();
            CHECK(forge::to_id(e) == i);
            CHECK(forge::to_version(e) == 0);
        }
    }

    TEST_CASE("world creates many entities with unique ids") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();
        auto e3 = world.make();

        CHECK(forge::to_id(e0) != forge::to_id(e1));
        CHECK(forge::to_id(e0) != forge::to_id(e2));
        CHECK(forge::to_id(e0) != forge::to_id(e3));

        CHECK(forge::to_id(e1) != forge::to_id(e2));
        CHECK(forge::to_id(e1) != forge::to_id(e3));

        CHECK(forge::to_id(e2) != forge::to_id(e3));
    }

    TEST_CASE("new entities start with version zero") {
        forge::registry<int, char, float> world;
        for (int i = 0; i < 100; i++) {
            auto e = world.make();
            CHECK(forge::to_version(e) == 0);
        }
    }

    TEST_CASE("destroying one entity does not destroy another") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();

        world.destroy(e1);

        CHECK(world.is_alive(e0));
        CHECK(!world.is_alive(e1));
        CHECK(world.is_alive(e2));
    }

    TEST_CASE("destroying multiple entities leaves remaining entities alive") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();
        auto e3 = world.make();
        auto e4 = world.make();

        world.destroy(e1);
        world.destroy(e3);

        CHECK(world.is_alive(e0));
        CHECK(!world.is_alive(e1));
        CHECK(world.is_alive(e2));
        CHECK(!world.is_alive(e3));
        CHECK(world.is_alive(e4));
    }

    // ============================================================
    // Entity reuse
    // ============================================================

    TEST_CASE("destroyed entity id is reused") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();

        world.destroy(e0);

        auto e2 = world.make();

        CHECK(forge::to_id(e2) == forge::to_id(e0));
        CHECK(forge::to_id(e2) != forge::to_id(e1));
    }

    TEST_CASE("reused entity gets incremented version") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();

        world.destroy(e0);

        auto e1 = world.make();

        CHECK(forge::to_id(e1) == forge::to_id(e0));
        CHECK(forge::to_version(e1) == forge::to_version(e0) + 1);
    }

    TEST_CASE("old entity remains dead after id is reused") {
        forge::registry<int, char, float> world;

        auto old = world.make();

        world.destroy(old);

        auto current = world.make();

        CHECK(forge::to_id(old) == forge::to_id(current));
        CHECK(forge::to_version(old) != forge::to_version(current));

        CHECK(!world.is_alive(old));
        CHECK(world.is_alive(current));
    }

    TEST_CASE("reusing an entity does not revive the old handle") {
        forge::registry<int, char, float> world;

        auto old = world.make();

        world.destroy(old);

        auto current = world.make();

        CHECK(!world.is_alive(old));
        CHECK(world.is_alive(current));
    }
    // ============================================================
    // Multiple reuse cycles
    // ============================================================

    TEST_CASE("entity version increments on every reuse") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();

        CHECK(forge::to_version(e0) == 0);

        world.destroy(e0);

        auto e1 = world.make();

        CHECK(forge::to_version(e1) == 1);

        world.destroy(e1);

        auto e2 = world.make();

        CHECK(forge::to_version(e2) == 2);

        world.destroy(e2);

        auto e3 = world.make();

        CHECK(forge::to_version(e3) == 3);
    }

    TEST_CASE("entity id remains stable across reuse") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        auto id = forge::to_id(e);

        for (int i = 1; i <= 10; i++) {
            world.destroy(e);

            e = world.make();

            CHECK(forge::to_id(e) == id);
            CHECK(forge::to_version(e) == i);
        }
    }

    TEST_CASE("multiple entities can be reused independently") {
        forge::registry<int, char, float> world;

        auto a = world.make();
        auto b = world.make();

        world.destroy(a);

        auto a2 = world.make();

        CHECK(forge::to_id(a2) == forge::to_id(a));
        CHECK(forge::to_version(a2) == 1);

        CHECK(forge::to_id(b) == 1);
        CHECK(forge::to_version(b) == 0);
        CHECK(world.is_alive(b));
    }

    TEST_CASE("destroying entities and reusing them preserves distinct ids") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        world.make();
        auto e2 = world.make();

        world.destroy(e0);
        world.destroy(e2);

        auto r0 = world.make();
        auto r2 = world.make();

        CHECK(forge::to_id(r0) != forge::to_id(r2));

        CHECK(forge::to_version(r0) == 1);
        CHECK(forge::to_version(r2) == 1);
    }

    // ============================================================
    // Reuse ordering
    // ============================================================

    TEST_CASE("destroyed entities are reused before new ids are allocated") {
        forge::registry<int, char, float> world;

        world.make();
        auto e1 = world.make();
        world.make();

        world.destroy(e1);

        auto e3 = world.make();

        CHECK(forge::to_id(e3) == forge::to_id(e1));
        CHECK(forge::to_version(e3) == 1);
    }

    TEST_CASE("all recycled entities are reused before allocating new ids") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();

        world.destroy(e0);
        world.destroy(e1);
        world.destroy(e2);

        auto r0 = world.make();
        auto r1 = world.make();
        auto r2 = world.make();

        CHECK(forge::to_version(r0) == 1);
        CHECK(forge::to_version(r1) == 1);
        CHECK(forge::to_version(r2) == 1);

        CHECK(forge::to_id(r0) != forge::to_id(r1));
        CHECK(forge::to_id(r1) != forge::to_id(r2));
        CHECK(forge::to_id(r0) != forge::to_id(r2));
    }

    TEST_CASE("destroyed entity is reused instead of increasing entity id") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();

        world.destroy(e0);

        auto e2 = world.make();

        CHECK(forge::to_id(e2) == 0);
        CHECK(forge::to_id(e1) == 1);
    }

    // ============================================================
    // Stale handles
    // ============================================================

    TEST_CASE("stale entity handle is not alive") {
        forge::registry<int, char, float> world;

        auto stale = world.make();

        world.destroy(stale);
        auto current = world.make();

        CHECK(!world.is_alive(stale));
        CHECK(world.is_alive(current));
    }

    TEST_CASE("stale handle remains invalid after several generations") {
        forge::registry<int, char, float> world;

        auto stale = world.make();

        world.destroy(stale);

        auto current = world.make();

        world.destroy(current);

        auto current2 = world.make();

        CHECK(!world.is_alive(stale));
        CHECK(!world.is_alive(current));
        CHECK(world.is_alive(current2));
    }

    TEST_CASE("each generation has a distinct entity value") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();

        world.destroy(e0);
        auto e1 = world.make();

        world.destroy(e1);
        auto e2 = world.make();

        CHECK(e0 != e1);
        CHECK(e1 != e2);
        CHECK(e0 != e2);
    }

    // ============================================================
    // Destroying in different orders
    // ============================================================

    TEST_CASE("destroying entities in reverse order") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();

        world.destroy(e2);
        world.destroy(e1);
        world.destroy(e0);

        CHECK(!world.is_alive(e0));
        CHECK(!world.is_alive(e1));
        CHECK(!world.is_alive(e2));

        auto r0 = world.make();
        auto r1 = world.make();
        auto r2 = world.make();

        CHECK(forge::to_version(r0) == 1);
        CHECK(forge::to_version(r1) == 1);
        CHECK(forge::to_version(r2) == 1);
    }

    TEST_CASE("destroying entities in arbitrary order") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();
        auto e3 = world.make();
        auto e4 = world.make();

        world.destroy(e3);
        world.destroy(e0);
        world.destroy(e4);
        world.destroy(e1);

        CHECK(!world.is_alive(e0));
        CHECK(!world.is_alive(e1));
        CHECK(world.is_alive(e2));
        CHECK(!world.is_alive(e3));
        CHECK(!world.is_alive(e4));

        auto r0 = world.make();
        auto r1 = world.make();
        auto r2 = world.make();
        auto r3 = world.make();

        CHECK(forge::to_version(r0) == 1);
        CHECK(forge::to_version(r1) == 1);
        CHECK(forge::to_version(r2) == 1);
        CHECK(forge::to_version(r3) == 1);
    }

    // ============================================================
    // Destroy / recreate with other entities alive
    // ============================================================

    TEST_CASE("recycling an entity does not affect other entities") {
        forge::registry<int, char, float> world;

        auto e0 = world.make();
        auto e1 = world.make();
        auto e2 = world.make();

        world.destroy(e1);

        auto e3 = world.make();

        CHECK(world.is_alive(e0));
        CHECK(world.is_alive(e2));
        CHECK(world.is_alive(e3));

        CHECK(forge::to_id(e3) == forge::to_id(e1));
        CHECK(forge::to_version(e3) == 1);

        CHECK(forge::to_id(e0) == 0);
        CHECK(forge::to_id(e2) == 2);
    }

    // ============================================================
    // Registry independence
    // ============================================================

    TEST_CASE("different registries have independent entity lifetimes") {
        forge::registry<int, char, float> r1;
        forge::registry<int, char, float> r2;

        auto e1 = r1.make();
        auto e2 = r2.make();

        CHECK(forge::to_id(e1) == 0);
        CHECK(forge::to_id(e2) == 0);

        CHECK(r1.is_alive(e1));
        CHECK(r2.is_alive(e2));

        r1.destroy(e1);

        CHECK(!r1.is_alive(e1));
        CHECK(r2.is_alive(e2));
    }

    TEST_CASE("different registries have independent generations") {
        forge::registry<int, char, float> r1;
        forge::registry<int, char, float> r2;

        auto e1 = r1.make();
        auto e2 = r2.make();

        r1.destroy(e1);

        auto e1_new = r1.make();

        CHECK(forge::to_id(e1_new) == 0);
        CHECK(forge::to_version(e1_new) == 1);

        CHECK(forge::to_id(e2) == 0);
        CHECK(forge::to_version(e2) == 0);

        CHECK(r2.is_alive(e2));
    }

    // ============================================================
    // Large-scale lifecycle tests
    // ============================================================

    TEST_CASE("world handles many entities") {
        forge::registry<int, char, float> world;

        constexpr int count = 1000;

        for (int i = 0; i < count; ++i) {
            auto e = world.make();

            CHECK(forge::to_id(e) == i);
            CHECK(forge::to_version(e) == 0);
            CHECK(world.is_alive(e));
        }
    }

    TEST_CASE("world handles many destroyed entities") {
        forge::registry<int, char, float> world;

        constexpr int count = 1000;

        std::vector<forge::entity> entities;

        for (int i = 0; i < count; ++i) {
            entities.push_back(world.make());
        }

        for (auto e : entities) {
            world.destroy(e);
            CHECK(!world.is_alive(e));
        }

        for (auto e : entities) {
            CHECK(!world.is_alive(e));
        }
    }

    TEST_CASE("world handles many generations") {
        forge::registry<int, char, float> world;

        auto e = world.make();

        for (int generation = 1; generation <= 1000; ++generation) {
            world.destroy(e);

            auto next = world.make();

            CHECK(forge::to_id(next) == forge::to_id(e));
            CHECK(forge::to_version(next) == generation);
            CHECK(world.is_alive(next));

            e = next;
        }
    }

    TEST_CASE("old generation is never alive after reuse") {
        forge::registry<int, char, float> world;

        auto old = world.make();

        world.destroy(old);

        auto current = world.make();

        CHECK(forge::to_id(old) == forge::to_id(current));
        CHECK(forge::to_version(old) + 1 == forge::to_version(current));

        CHECK(!world.is_alive(old));
        CHECK(world.is_alive(current));
    }
}
TEST_CASE("world creates/destroys/reuses entity") {
    forge::registry<int, char, float> world;
    auto e = world.make();
    CHECK(forge::to_id(e) == 0);
    world.destroy(e);
    CHECK(!world.is_alive(e));
    auto e2 = world.make();
    CHECK(forge::to_id(e2) == 0);
    CHECK(forge::to_version(e2) == 1);
}

TEST_CASE("world creates multiple entities") {
    forge::registry<int, char, float> world;
    for (int i = 0; i < 10; i++) {
        auto e = world.make();
        CHECK(forge::to_id(e) == i);
        CHECK(forge::to_version(e) == 0);
    }
}

TEST_CASE("world creates many entities with unique ids") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();
    auto e2 = world.make();
    auto e3 = world.make();

    CHECK(forge::to_id(e0) != forge::to_id(e1));
    CHECK(forge::to_id(e0) != forge::to_id(e2));
    CHECK(forge::to_id(e0) != forge::to_id(e3));

    CHECK(forge::to_id(e1) != forge::to_id(e2));
    CHECK(forge::to_id(e1) != forge::to_id(e3));

    CHECK(forge::to_id(e2) != forge::to_id(e3));
}

TEST_CASE("new entities start with version zero") {
    forge::registry<int, char, float> world;
    for (int i = 0; i < 100; i++) {
        auto e = world.make();
        CHECK(forge::to_version(e) == 0);
    }
}

TEST_CASE("destroying one entity does not destroy another") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();
    auto e2 = world.make();

    world.destroy(e1);

    CHECK(world.is_alive(e0));
    CHECK(!world.is_alive(e1));
    CHECK(world.is_alive(e2));
}

TEST_CASE("destroying multiple entities leaves remaining entities alive") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();
    auto e2 = world.make();
    auto e3 = world.make();
    auto e4 = world.make();

    world.destroy(e1);
    world.destroy(e3);

    CHECK(world.is_alive(e0));
    CHECK(!world.is_alive(e1));
    CHECK(world.is_alive(e2));
    CHECK(!world.is_alive(e3));
    CHECK(world.is_alive(e4));
}

// ============================================================
// Entity reuse
// ============================================================

TEST_CASE("destroyed entity id is reused") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();

    world.destroy(e0);

    auto e2 = world.make();

    CHECK(forge::to_id(e2) == forge::to_id(e0));
    CHECK(forge::to_id(e2) != forge::to_id(e1));
}

TEST_CASE("reused entity gets incremented version") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();

    world.destroy(e0);

    auto e1 = world.make();

    CHECK(forge::to_id(e1) == forge::to_id(e0));
    CHECK(forge::to_version(e1) == forge::to_version(e0) + 1);
}

TEST_CASE("old entity remains dead after id is reused") {
    forge::registry<int, char, float> world;

    auto old = world.make();

    world.destroy(old);

    auto current = world.make();

    CHECK(forge::to_id(old) == forge::to_id(current));
    CHECK(forge::to_version(old) != forge::to_version(current));

    CHECK(!world.is_alive(old));
    CHECK(world.is_alive(current));
}

TEST_CASE("reusing an entity does not revive the old handle") {
    forge::registry<int, char, float> world;

    auto old = world.make();

    world.destroy(old);

    auto current = world.make();

    CHECK(!world.is_alive(old));
    CHECK(world.is_alive(current));
}
// ============================================================
// Multiple reuse cycles
// ============================================================

TEST_CASE("entity version increments on every reuse") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();

    CHECK(forge::to_version(e0) == 0);

    world.destroy(e0);

    auto e1 = world.make();

    CHECK(forge::to_version(e1) == 1);

    world.destroy(e1);

    auto e2 = world.make();

    CHECK(forge::to_version(e2) == 2);

    world.destroy(e2);

    auto e3 = world.make();

    CHECK(forge::to_version(e3) == 3);
}

TEST_CASE("entity id remains stable across reuse") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    auto id = forge::to_id(e);

    for (int i = 1; i <= 10; i++) {
        world.destroy(e);

        e = world.make();

        CHECK(forge::to_id(e) == id);
        CHECK(forge::to_version(e) == i);
    }
}

TEST_CASE("multiple entities can be reused independently") {
    forge::registry<int, char, float> world;

    auto a = world.make();
    auto b = world.make();

    world.destroy(a);

    auto a2 = world.make();

    CHECK(forge::to_id(a2) == forge::to_id(a));
    CHECK(forge::to_version(a2) == 1);

    CHECK(forge::to_id(b) == 1);
    CHECK(forge::to_version(b) == 0);
    CHECK(world.is_alive(b));
}

TEST_CASE("destroying entities and reusing them preserves distinct ids") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    world.make();
    auto e2 = world.make();

    world.destroy(e0);
    world.destroy(e2);

    auto r0 = world.make();
    auto r2 = world.make();

    CHECK(forge::to_id(r0) != forge::to_id(r2));

    CHECK(forge::to_version(r0) == 1);
    CHECK(forge::to_version(r2) == 1);
}

// ============================================================
// Reuse ordering
// ============================================================

TEST_CASE("destroyed entities are reused before new ids are allocated") {
    forge::registry<int, char, float> world;

    world.make();
    auto e1 = world.make();
    world.make();

    world.destroy(e1);

    auto e3 = world.make();

    CHECK(forge::to_id(e3) == forge::to_id(e1));
    CHECK(forge::to_version(e3) == 1);
}

TEST_CASE("all recycled entities are reused before allocating new ids") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();
    auto e2 = world.make();

    world.destroy(e0);
    world.destroy(e1);
    world.destroy(e2);

    auto r0 = world.make();
    auto r1 = world.make();
    auto r2 = world.make();

    CHECK(forge::to_version(r0) == 1);
    CHECK(forge::to_version(r1) == 1);
    CHECK(forge::to_version(r2) == 1);

    CHECK(forge::to_id(r0) != forge::to_id(r1));
    CHECK(forge::to_id(r1) != forge::to_id(r2));
    CHECK(forge::to_id(r0) != forge::to_id(r2));
}

TEST_CASE("destroyed entity is reused instead of increasing entity id") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();

    world.destroy(e0);

    auto e2 = world.make();

    CHECK(forge::to_id(e2) == 0);
    CHECK(forge::to_id(e1) == 1);
}

// ============================================================
// Stale handles
// ============================================================

TEST_CASE("stale entity handle is not alive") {
    forge::registry<int, char, float> world;

    auto stale = world.make();

    world.destroy(stale);
    auto current = world.make();

    CHECK(!world.is_alive(stale));
    CHECK(world.is_alive(current));
}

TEST_CASE("stale handle remains invalid after several generations") {
    forge::registry<int, char, float> world;

    auto stale = world.make();

    world.destroy(stale);

    auto current = world.make();

    world.destroy(current);

    auto current2 = world.make();

    CHECK(!world.is_alive(stale));
    CHECK(!world.is_alive(current));
    CHECK(world.is_alive(current2));
}

TEST_CASE("each generation has a distinct entity value") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();

    world.destroy(e0);
    auto e1 = world.make();

    world.destroy(e1);
    auto e2 = world.make();

    CHECK(e0 != e1);
    CHECK(e1 != e2);
    CHECK(e0 != e2);
}

// ============================================================
// Destroying in different orders
// ============================================================

TEST_CASE("destroying entities in reverse order") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();
    auto e2 = world.make();

    world.destroy(e2);
    world.destroy(e1);
    world.destroy(e0);

    CHECK(!world.is_alive(e0));
    CHECK(!world.is_alive(e1));
    CHECK(!world.is_alive(e2));

    auto r0 = world.make();
    auto r1 = world.make();
    auto r2 = world.make();

    CHECK(forge::to_version(r0) == 1);
    CHECK(forge::to_version(r1) == 1);
    CHECK(forge::to_version(r2) == 1);
}

TEST_CASE("destroying entities in arbitrary order") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();
    auto e2 = world.make();
    auto e3 = world.make();
    auto e4 = world.make();

    world.destroy(e3);
    world.destroy(e0);
    world.destroy(e4);
    world.destroy(e1);

    CHECK(!world.is_alive(e0));
    CHECK(!world.is_alive(e1));
    CHECK(world.is_alive(e2));
    CHECK(!world.is_alive(e3));
    CHECK(!world.is_alive(e4));

    auto r0 = world.make();
    auto r1 = world.make();
    auto r2 = world.make();
    auto r3 = world.make();

    CHECK(forge::to_version(r0) == 1);
    CHECK(forge::to_version(r1) == 1);
    CHECK(forge::to_version(r2) == 1);
    CHECK(forge::to_version(r3) == 1);
}

// ============================================================
// Destroy / recreate with other entities alive
// ============================================================

TEST_CASE("recycling an entity does not affect other entities") {
    forge::registry<int, char, float> world;

    auto e0 = world.make();
    auto e1 = world.make();
    auto e2 = world.make();

    world.destroy(e1);

    auto e3 = world.make();

    CHECK(world.is_alive(e0));
    CHECK(world.is_alive(e2));
    CHECK(world.is_alive(e3));

    CHECK(forge::to_id(e3) == forge::to_id(e1));
    CHECK(forge::to_version(e3) == 1);

    CHECK(forge::to_id(e0) == 0);
    CHECK(forge::to_id(e2) == 2);
}

// ============================================================
// Registry independence
// ============================================================

TEST_CASE("different registries have independent entity lifetimes") {
    forge::registry<int, char, float> r1;
    forge::registry<int, char, float> r2;

    auto e1 = r1.make();
    auto e2 = r2.make();

    CHECK(forge::to_id(e1) == 0);
    CHECK(forge::to_id(e2) == 0);

    CHECK(r1.is_alive(e1));
    CHECK(r2.is_alive(e2));

    r1.destroy(e1);

    CHECK(!r1.is_alive(e1));
    CHECK(r2.is_alive(e2));
}

TEST_CASE("different registries have independent generations") {
    forge::registry<int, char, float> r1;
    forge::registry<int, char, float> r2;

    auto e1 = r1.make();
    auto e2 = r2.make();

    r1.destroy(e1);

    auto e1_new = r1.make();

    CHECK(forge::to_id(e1_new) == 0);
    CHECK(forge::to_version(e1_new) == 1);

    CHECK(forge::to_id(e2) == 0);
    CHECK(forge::to_version(e2) == 0);

    CHECK(r2.is_alive(e2));
}

// ============================================================
// Large-scale lifecycle tests
// ============================================================

TEST_CASE("world handles many entities") {
    forge::registry<int, char, float> world;

    constexpr int count = 1000;

    for (int i = 0; i < count; ++i) {
        auto e = world.make();

        CHECK(forge::to_id(e) == i);
        CHECK(forge::to_version(e) == 0);
        CHECK(world.is_alive(e));
    }
}

TEST_CASE("world handles many destroyed entities") {
    forge::registry<int, char, float> world;

    constexpr int count = 1000;

    std::vector<forge::entity> entities;

    for (int i = 0; i < count; ++i) {
        entities.push_back(world.make());
    }

    for (auto e : entities) {
        world.destroy(e);
        CHECK(!world.is_alive(e));
    }

    for (auto e : entities) {
        CHECK(!world.is_alive(e));
    }
}

TEST_CASE("world handles many generations") {
    forge::registry<int, char, float> world;

    auto e = world.make();

    for (int generation = 1; generation <= 1000; ++generation) {
        world.destroy(e);

        auto next = world.make();

        CHECK(forge::to_id(next) == forge::to_id(e));
        CHECK(forge::to_version(next) == generation);
        CHECK(world.is_alive(next));

        e = next;
    }
}

TEST_CASE("old generation is never alive after reuse") {
    forge::registry<int, char, float> world;

    auto old = world.make();

    world.destroy(old);

    auto current = world.make();

    CHECK(forge::to_id(old) == forge::to_id(current));
    CHECK(forge::to_version(old) + 1 == forge::to_version(current));

    CHECK(!world.is_alive(old));
    CHECK(world.is_alive(current));
}
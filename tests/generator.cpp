#include <forge/forge.hpp>
#include <iostream>

#include "doctest.h"

TEST_SUITE("entity generator") {
    using u64_generator = forge::generator<std::uint64_t>;
    using u32_generator = forge::generator<std::uint32_t>;

    TEST_CASE("creates first entity") {
        u64_generator generator;

        auto e = generator.make();

        CHECK(forge::to_entity(e) == 0);
        CHECK(forge::to_version(e) == 0);
        CHECK(generator.valid(e));
    }

    TEST_CASE("creates entities with sequential ids") {
        u64_generator generator;

        auto e0 = generator.make();
        auto e1 = generator.make();
        auto e2 = generator.make();
        auto e3 = generator.make();

        CHECK(forge::to_entity(e0) == 0);
        CHECK(forge::to_entity(e1) == 1);
        CHECK(forge::to_entity(e2) == 2);
        CHECK(forge::to_entity(e3) == 3);

        CHECK(forge::to_version(e0) == 0);
        CHECK(forge::to_version(e1) == 0);
        CHECK(forge::to_version(e2) == 0);
        CHECK(forge::to_version(e3) == 0);
    }

    TEST_CASE("fresh entities are valid") {
        u64_generator generator;

        auto e0 = generator.make();
        auto e1 = generator.make();
        auto e2 = generator.make();

        CHECK(generator.valid(e0));
        CHECK(generator.valid(e1));
        CHECK(generator.valid(e2));
    }

    TEST_CASE("destroy invalidates entity") {
        u64_generator generator;

        auto e = generator.make();

        CHECK(generator.valid(e));

        generator.destroy(e);

        CHECK_FALSE(generator.valid(e));
    }

    TEST_CASE("destroy increments version") {
        u64_generator generator;

        auto e0 = generator.make();

        CHECK(forge::to_entity(e0) == 0);
        CHECK(forge::to_version(e0) == 0);

        generator.destroy(e0);

        auto e1 = generator.make();

        CHECK(forge::to_entity(e1) == 0);
        CHECK(forge::to_version(e1) == 1);
        CHECK(generator.valid(e1));
    }

    TEST_CASE("old entity remains invalid after recycling") {
        u64_generator generator;

        auto e0 = generator.make();

        generator.destroy(e0);

        auto e1 = generator.make();

        CHECK(e0 != e1);
        CHECK_FALSE(generator.valid(e0));
        CHECK(generator.valid(e1));
    }

    TEST_CASE("recycling preserves entity id") {
        u64_generator generator;

        auto e0 = generator.make();
        const auto id = forge::to_entity(e0);

        generator.destroy(e0);

        auto e1 = generator.make();

        CHECK(forge::to_entity(e1) == id);
    }

    TEST_CASE("multiple entities recycle independently") {
        u64_generator generator;

        auto e0 = generator.make();
        auto e1 = generator.make();
        auto e2 = generator.make();

        generator.destroy(e0);
        generator.destroy(e1);

        auto r0 = generator.make();
        auto r1 = generator.make();

        CHECK(forge::to_entity(r0) == forge::to_entity(e1));
        CHECK(forge::to_entity(r1) == forge::to_entity(e0));

        CHECK(forge::to_version(r0) == 1);
        CHECK(forge::to_version(r1) == 1);

        CHECK_FALSE(generator.valid(e0));
        CHECK_FALSE(generator.valid(e1));
        CHECK(generator.valid(r0));
        CHECK(generator.valid(r1));

        CHECK(generator.valid(e2));
    }

    TEST_CASE("destroying one entity does not invalidate others") {
        u64_generator generator;

        auto e0 = generator.make();
        auto e1 = generator.make();
        auto e2 = generator.make();

        generator.destroy(e1);

        CHECK(generator.valid(e0));
        CHECK_FALSE(generator.valid(e1));
        CHECK(generator.valid(e2));
    }

    TEST_CASE("double destruction does nothing") {
        u64_generator generator;

        auto e = generator.make();

        generator.destroy(e);

        // e is already invalid.
        generator.destroy(e);

        auto recycled = generator.make();

        CHECK(forge::to_entity(recycled) == forge::to_entity(e));
        CHECK(forge::to_version(recycled) == 1);
        CHECK(generator.valid(recycled));
    }

    TEST_CASE("destroying an invalid entity does nothing") {
        u64_generator generator;
        auto e = generator.make();
        generator.destroy(e);
        auto recycled = generator.make();
        generator.destroy(e);
        auto next = generator.make();
        CHECK(generator.valid(recycled));
        CHECK(!generator.valid(e));
        CHECK(forge::to_version(recycled) == 1);
        CHECK(forge::to_entity(next) != forge::to_entity(recycled));
        CHECK(forge::to_version(next) != 2);
    }

    TEST_CASE("destroying arbitrary invalid value does nothing") {
        u64_generator generator;

        auto e0 = generator.make();
        auto e1 = generator.make();

        const auto invalid =
            forge::entity_traits<std::uint64_t>::construct(999, 0);

        CHECK_FALSE(generator.valid(invalid));

        generator.destroy(invalid);

        CHECK(generator.valid(e0));
        CHECK(generator.valid(e1));
    }

    TEST_CASE("version history tracks each entity") {
        u64_generator generator;

        auto e0 = generator.make();
        auto e1 = generator.make();
        auto e2 = generator.make();

        generator.destroy(e0);

        auto r0 = generator.make();

        generator.destroy(e2);

        auto r2 = generator.make();

        CHECK(forge::to_entity(r0) == forge::to_entity(e0));
        CHECK(forge::to_version(r0) == 1);

        CHECK(forge::to_entity(r2) == forge::to_entity(e2));
        CHECK(forge::to_version(r2) == 1);

        CHECK(generator.valid(e1));
        CHECK(generator.valid(r0));
        CHECK(generator.valid(r2));

        CHECK_FALSE(generator.valid(e0));
        CHECK_FALSE(generator.valid(e2));
    }

    TEST_CASE("large number of fresh entities") {
        u64_generator generator;

        constexpr std::size_t count = 1000;

        std::vector<std::uint64_t> entities;
        entities.reserve(count);

        for (std::size_t i = 0; i < count; ++i) {
            entities.push_back(generator.make());
        }

        for (std::size_t i = 0; i < count; ++i) {
            CHECK(forge::to_entity(entities[i]) == i);
            CHECK(forge::to_version(entities[i]) == 0);
            CHECK(generator.valid(entities[i]));
        }
    }

    TEST_CASE("large number of recycled entities") {
        u64_generator generator;

        constexpr std::size_t count = 1000;

        std::vector<std::uint64_t> entities;
        entities.reserve(count);

        for (std::size_t i = 0; i < count; ++i) {
            entities.push_back(generator.make());
        }

        for (auto e : entities) {
            generator.destroy(e);
        }

        for (auto e : entities) {
            CHECK_FALSE(generator.valid(e));
        }

        std::vector<std::uint64_t> recycled;
        recycled.reserve(count);

        for (std::size_t i = 0; i < count; ++i) {
            recycled.push_back(generator.make());
        }

        for (auto e : recycled) {
            CHECK(generator.valid(e));
            CHECK(forge::to_version(e) == 1);
        }
    }

    TEST_CASE("repeated recycling increments version") {
        u64_generator generator;

        auto e0 = generator.make();

        CHECK(forge::to_version(e0) == 0);
        CHECK(generator.valid(e0));

        generator.destroy(e0);

        auto e1 = generator.make();

        CHECK(forge::to_version(e1) == 1);
        CHECK(generator.valid(e1));
        CHECK_FALSE(generator.valid(e0));

        generator.destroy(e1);

        auto e2 = generator.make();

        CHECK(forge::to_version(e2) == 2);
        CHECK(generator.valid(e2));
        CHECK_FALSE(generator.valid(e1));

        generator.destroy(e2);

        auto e3 = generator.make();

        CHECK(forge::to_version(e3) == 3);
        CHECK(generator.valid(e3));
        CHECK_FALSE(generator.valid(e2));
    }

    TEST_CASE("uint32 entity representation works") {
        u32_generator generator;

        auto e0 = generator.make();
        auto e1 = generator.make();

        CHECK(forge::to_entity(e0) == 0);
        CHECK(forge::to_entity(e1) == 1);

        CHECK(forge::to_version(e0) == 0);
        CHECK(forge::to_version(e1) == 0);

        CHECK(generator.valid(e0));
        CHECK(generator.valid(e1));

        generator.destroy(e0);

        auto recycled = generator.make();

        CHECK(forge::to_entity(recycled) == 0);
        CHECK(forge::to_version(recycled) == 1);

        CHECK_FALSE(generator.valid(e0));
        CHECK(generator.valid(recycled));
        CHECK(generator.valid(e1));
    }

    TEST_CASE("move construction preserves generator state") {
        u64_generator generator;

        auto e0 = generator.make();
        auto e1 = generator.make();

        generator.destroy(e0);

        u64_generator moved{std::move(generator)};

        CHECK(moved.valid(e1));
        CHECK_FALSE(moved.valid(e0));

        auto recycled = moved.make();

        CHECK(forge::to_entity(recycled) == forge::to_entity(e0));
        CHECK(forge::to_version(recycled) == 1);
        CHECK(moved.valid(recycled));
    }

    TEST_CASE("copy construction copies generator state") {
        u64_generator generator;

        auto e0 = generator.make();
        auto e1 = generator.make();

        generator.destroy(e0);

        u64_generator copy{generator};

        CHECK(copy.valid(e1));
        CHECK_FALSE(copy.valid(e0));

        auto original_recycled = generator.make();
        auto copy_recycled = copy.make();

        CHECK(forge::to_version(original_recycled) == 1);
        CHECK(forge::to_version(copy_recycled) == 1);
    }
}
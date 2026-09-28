#include <chrono>
#include <cstddef>
#include <cstdint>
#include <entt/entt.hpp>
#include <forge/forge.hpp>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace benchmark {

using clock = std::chrono::steady_clock;

constexpr std::size_t ENTITY_COUNT = 1'000'000;

struct Position {
    float x;
    float y;
};

struct Velocity {
    float x;
    float y;
};

struct Health {
    int value;
};

struct Name {
    std::string value;
};

volatile std::uint64_t sink = 0;

template <typename Fn>
double run(const char* name, Fn&& fn) {
    const auto start = clock::now();

    fn();

    const auto end = clock::now();

    const double ms =
        std::chrono::duration<double, std::milli>(end - start).count();

    std::cout
        << std::left
        << std::setw(40)
        << name
        << std::right
        << std::setw(12)
        << std::fixed
        << std::setprecision(3)
        << ms
        << " ms\n";

    return ms;
}

}  // namespace benchmark

// ================================================================
// Entity creation
// ================================================================

void benchmark_entity_creation() {
    using namespace benchmark;

    std::cout << "\n=== Entity Creation ===\n";

    {
        forge::registry<
            Position,
            Velocity,
            Health,
            Name>
            world;

        run("Forge", [&] {
            for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
                sink += forge::to_id(world.make());
            }
        });
    }

    {
        entt::registry world;

        run("EnTT", [&] {
            for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
                sink += static_cast<std::uint32_t>(entt::to_entity(world.create()));
            }
        });
    }
}

// ================================================================
// Entity + component creation
// ================================================================

void benchmark_emplace() {
    using namespace benchmark;

    std::cout << "\n=== Entity + Component Creation ===\n";

    {
        forge::registry<
            Position,
            Velocity,
            Health,
            Name>
            world;

        run("Forge", [&] {
            for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
                const auto e = world.make();

                world.add_component<Position>(
                    e,
                    static_cast<float>(i),
                    static_cast<float>(i));

                world.add_component<Velocity>(
                    e,
                    1.0f,
                    1.0f);

                world.add_component<Health>(
                    e,
                    100);

                world.add_component<Name>(
                    e,
                    "entity");
            }
        });
    }

    {
        entt::registry world;

        run("EnTT", [&] {
            for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
                const auto e = world.create();

                world.emplace<Position>(
                    e,
                    static_cast<float>(i),
                    static_cast<float>(i));

                world.emplace<Velocity>(
                    e,
                    1.0f,
                    1.0f);

                world.emplace<Health>(
                    e,
                    100);

                world.emplace<Name>(
                    e,
                    "entity");
            }
        });
    }
}

// ================================================================
// Direct view iteration
// ================================================================

void benchmark_view_iteration() {
    using namespace benchmark;

    std::cout << "\n=== View Iteration ===\n";

    {
        forge::registry<
            Position,
            Velocity,
            Health,
            Name>
            world;

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            const auto e = world.make();

            world.add_component<Position>(
                e,
                static_cast<float>(i),
                static_cast<float>(i));

            world.add_component<Velocity>(
                e,
                1.0f,
                1.0f);

            world.add_component<Health>(e, 100);
        }

        auto view = world.view<Position, Velocity>();

        run("Forge", [&] {
            view.each([](Position& position, Velocity& velocity) {
                position.x += velocity.x;
                position.y += velocity.y;

                sink += static_cast<std::uint64_t>(position.x);
            });
        });
    }

    {
        entt::registry world;

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            const auto e = world.create();

            world.emplace<Position>(
                e,
                static_cast<float>(i),
                static_cast<float>(i));

            world.emplace<Velocity>(
                e,
                1.0f,
                1.0f);

            world.emplace<Health>(e, 100);
        }

        auto view = world.view<Position, Velocity>();

        run("EnTT", [&] {
            view.each([](Position& position, Velocity& velocity) {
                position.x += velocity.x;
                position.y += velocity.y;

                sink += static_cast<std::uint64_t>(position.x);
            });
        });
    }
}

// ================================================================
// View iteration with entity
// ================================================================

void benchmark_view_iteration_with_entity() {
    using namespace benchmark;

    std::cout << "\n=== View Iteration + Entity ===\n";

    {
        forge::registry<
            Position,
            Velocity,
            Health>
            world;

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            const auto e = world.make();

            world.add_component<Position>(
                e,
                static_cast<float>(i),
                static_cast<float>(i));

            world.add_component<Velocity>(
                e,
                1.0f,
                1.0f);

            world.add_component<Health>(e, 100);
        }

        auto view = world.view<Position, Velocity>();

        run("Forge", [&] {
            view.each([](
                          const forge::entity e,
                          Position& position,
                          Velocity& velocity) {
                position.x += velocity.x;

                sink += forge::to_id(e);
            });
        });
    }

    {
        entt::registry world;

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            const auto e = world.create();

            world.emplace<Position>(
                e,
                static_cast<float>(i),
                static_cast<float>(i));

            world.emplace<Velocity>(
                e,
                1.0f,
                1.0f);

            world.emplace<Health>(e, 100);
        }

        auto view = world.view<Position, Velocity>();

        run("EnTT", [&] {
            view.each([](
                          const entt::entity e,
                          Position& position,
                          Velocity& velocity) {
                position.x += velocity.x;

                sink += entt::to_entity(e);
            });
        });
    }
}

// ================================================================
// Immutable view
// ================================================================

void benchmark_const_view() {
    using namespace benchmark;

    std::cout << "\n=== Const View Iteration ===\n";

    {
        forge::registry<
            Position,
            Velocity>
            world;

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            const auto e = world.make();

            world.add_component<Position>(
                e,
                static_cast<float>(i),
                2.0f);

            world.add_component<Velocity>(
                e,
                1.0f,
                1.0f);
        }

        auto view = world.view<const Position, const Velocity>();

        run("Forge", [&] {
            view.each([](
                          const Position& position,
                          const Velocity& velocity) {
                sink += static_cast<std::uint64_t>(
                    position.x + velocity.x);
            });
        });
    }

    {
        entt::registry world;

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            const auto e = world.create();

            world.emplace<Position>(
                e,
                static_cast<float>(i),
                2.0f);

            world.emplace<Velocity>(
                e,
                1.0f,
                1.0f);
        }

        auto view = world.view<const Position, const Velocity>();

        run("EnTT", [&] {
            view.each([](
                          const Position& position,
                          const Velocity& velocity) {
                sink += static_cast<std::uint64_t>(
                    position.x + velocity.x);
            });
        });
    }
}

// ================================================================
// Component lookup
// ================================================================

void benchmark_component_lookup() {
    using namespace benchmark;

    std::cout << "\n=== Component Lookup ===\n";

    {
        forge::registry<Position, Velocity> world;

        std::vector<forge::entity> entities;
        entities.reserve(ENTITY_COUNT);

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            auto e = world.make();

            world.add_component<Position>(
                e,
                static_cast<float>(i),
                0.0f);

            entities.push_back(e);
        }

        run("Forge", [&] {
            for (auto e : entities) {
                sink += static_cast<std::uint64_t>(
                    world.get_component<Position>(e).x);
            }
        });
    }

    {
        entt::registry world;

        std::vector<entt::entity> entities;
        entities.reserve(ENTITY_COUNT);

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            auto e = world.create();

            world.emplace<Position>(
                e,
                static_cast<float>(i),
                0.0f);

            entities.push_back(e);
        }

        run("EnTT", [&] {
            for (auto e : entities) {
                sink += static_cast<std::uint64_t>(
                    world.get<Position>(e).x);
            }
        });
    }
}

// ================================================================
// Destruction
// ================================================================

void benchmark_destruction() {
    using namespace benchmark;

    std::cout << "\n=== Entity Destruction ===\n";

    {
        forge::registry<
            Position,
            Velocity,
            Health>
            world;

        std::vector<forge::entity> entities;
        entities.reserve(ENTITY_COUNT);

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            auto e = world.make();

            world.add_component<Position>(e, 1.0f, 2.0f);
            world.add_component<Velocity>(e, 1.0f, 2.0f);
            world.add_component<Health>(e, 100);

            entities.push_back(e);
        }

        run("Forge", [&] {
            for (auto e : entities) {
                world.destroy(e);
            }
        });
    }

    {
        entt::registry world;

        std::vector<entt::entity> entities;
        entities.reserve(ENTITY_COUNT);

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            auto e = world.create();

            world.emplace<Position>(e, 1.0f, 2.0f);
            world.emplace<Velocity>(e, 1.0f, 2.0f);
            world.emplace<Health>(e, 100);

            entities.push_back(e);
        }

        run("EnTT", [&] {
            for (auto e : entities) {
                world.destroy(e);
            }
        });
    }
}

// ================================================================
// Entity recycling
// ================================================================

void benchmark_recycling() {
    using namespace benchmark;

    std::cout << "\n=== Entity Recycling ===\n";

    {
        forge::registry<Position, Health> world;

        std::vector<forge::entity> entities;
        entities.reserve(ENTITY_COUNT);

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            auto e = world.make();

            world.add_component<Position>(
                e,
                static_cast<float>(i),
                0.0f);

            world.add_component<Health>(e, 100);

            entities.push_back(e);
        }

        run("Forge", [&] {
            for (auto e : entities) {
                world.destroy(e);
            }

            for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
                auto e = world.make();

                world.add_component<Position>(
                    e,
                    static_cast<float>(i),
                    0.0f);

                world.add_component<Health>(e, 100);
            }
        });
    }

    {
        entt::registry world;

        std::vector<entt::entity> entities;
        entities.reserve(ENTITY_COUNT);

        for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
            auto e = world.create();

            world.emplace<Position>(
                e,
                static_cast<float>(i),
                0.0f);

            world.emplace<Health>(e, 100);

            entities.push_back(e);
        }

        run("EnTT", [&] {
            for (auto e : entities) {
                world.destroy(e);
            }

            for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
                auto e = world.create();

                world.emplace<Position>(
                    e,
                    static_cast<float>(i),
                    0.0f);

                world.emplace<Health>(e, 100);
            }
        });
    }
}

// ================================================================
// Main
// ================================================================

int main() {
    std::cout
        << "============================================\n"
        << "        Forge vs EnTT Benchmark\n"
        << "============================================\n"
        << "Entities: " << benchmark::ENTITY_COUNT << "\n";

    benchmark_entity_creation();
    benchmark_emplace();

    benchmark_view_iteration();
    benchmark_view_iteration_with_entity();
    benchmark_const_view();

    benchmark_component_lookup();

    benchmark_destruction();
    benchmark_recycling();

    std::cout << "\nSink: " << benchmark::sink << '\n';

    return 0;
}
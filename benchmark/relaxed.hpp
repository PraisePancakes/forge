#pragma once
#include <entt/entt.hpp>
#include <forge/forge.hpp>

#include "benchmark.hpp"
#include "components.hpp"

namespace benchmark {
class relaxed_benchmark : public benchmark {
    configuration _config;

   public:
    void run() override {
        print_header("Forge vs EnTT - Relaxed Benchmark");

        std::cout
            << "Entities: "
            << _config.initial_entities
            << '\n';

        entity_creation();
        entity_component_creation();
        view_iteration();
        view_iteration_with_entity();
        const_view_iteration();
        component_lookup();
        destruction();
        recycling();

        print_sink();
    }
    relaxed_benchmark(configuration config = {}) : _config{config} {};

   private:
    void entity_creation() {
        std::cout << "\n=== Entity Creation ===\n";

        {
            forge::registry<
                Position,
                Velocity,
                Health,
                Name>
                world;

            measure("Forge", [&] {
                for (std::size_t i = 0;
                     i < _config.initial_entities;
                     ++i) {
                    sink += forge::to_id(world.make());
                }
            });
        }

        {
            entt::registry world;

            measure("EnTT", [&] {
                for (std::size_t i = 0;
                     i < _config.initial_entities;
                     ++i) {
                    sink += static_cast<std::uint32_t>(
                        entt::to_entity(world.create()));
                }
            });
        }
    }
    void entity_component_creation() {
        std::cout
            << "\n=== Entity + Component Creation ===\n";

        {
            forge::registry<
                Position,
                Velocity,
                Health,
                Name>
                world;

            measure("Forge", [&] {
                for (std::size_t i = 0;
                     i < _config.initial_entities;
                     ++i) {
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

            measure("EnTT", [&] {
                for (std::size_t i = 0;
                     i < _config.initial_entities;
                     ++i) {
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
    void view_iteration() {
        std::cout << "\n=== View Iteration ===\n";

        {
            forge::registry<
                Position,
                Velocity,
                Health>
                world;

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
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
            }

            auto view = world.view<Position, Velocity>();

            measure("Forge", [&] {
                view.each([](
                              Position& position,
                              Velocity& velocity) {
                    position.x += velocity.x;
                    position.y += velocity.y;

                    sink += static_cast<std::uint64_t>(
                        position.x);
                });
            });
        }

        {
            entt::registry world;

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
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
            }

            auto view = world.view<Position, Velocity>();

            measure("EnTT", [&] {
                view.each([](
                              Position& position,
                              Velocity& velocity) {
                    position.x += velocity.x;
                    position.y += velocity.y;

                    sink += static_cast<std::uint64_t>(
                        position.x);
                });
            });
        }
    }
    void view_iteration_with_entity() {
        std::cout
            << "\n=== View Iteration + Entity ===\n";

        {
            forge::registry<
                Position,
                Velocity,
                Health>
                world;

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
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
            }

            auto view = world.view<Position, Velocity>();

            measure("Forge", [&] {
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

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
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
            }

            auto view = world.view<Position, Velocity>();

            measure("EnTT", [&] {
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
    void const_view_iteration() {
        std::cout
            << "\n=== Const View Iteration ===\n";

        {
            forge::registry<
                Position,
                Velocity>
                world;

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
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

            auto view =
                world.view<
                    const Position,
                    const Velocity>();

            measure("Forge", [&] {
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

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
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

            auto view =
                world.view<
                    const Position,
                    const Velocity>();

            measure("EnTT", [&] {
                view.each([](
                              const Position& position,
                              const Velocity& velocity) {
                    sink += static_cast<std::uint64_t>(
                        position.x + velocity.x);
                });
            });
        }
    }
    void component_lookup() {
        std::cout << "\n=== Component Lookup ===\n";

        {
            forge::registry<
                Position,
                Velocity>
                world;

            std::vector<forge::entity> entities;
            entities.reserve(_config.initial_entities);

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
                auto e = world.make();

                world.add_component<Position>(
                    e,
                    static_cast<float>(i),
                    0.0f);

                entities.push_back(e);
            }

            measure("Forge", [&] {
                for (auto e : entities) {
                    sink += static_cast<std::uint64_t>(
                        world.get_component<Position>(e).x);
                }
            });
        }

        {
            entt::registry world;

            std::vector<entt::entity> entities;
            entities.reserve(_config.initial_entities);

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
                auto e = world.create();

                world.emplace<Position>(
                    e,
                    static_cast<float>(i),
                    0.0f);

                entities.push_back(e);
            }

            measure("EnTT", [&] {
                for (auto e : entities) {
                    sink += static_cast<std::uint64_t>(
                        world.get<Position>(e).x);
                }
            });
        }
    }
    void destruction() {
        std::cout
            << "\n=== Entity Destruction ===\n";

        {
            forge::registry<
                Position,
                Velocity,
                Health>
                world;

            std::vector<forge::entity> entities;
            entities.reserve(_config.initial_entities);

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
                auto e = world.make();

                world.add_component<Position>(
                    e, 1.0f, 2.0f);

                world.add_component<Velocity>(
                    e, 1.0f, 2.0f);

                world.add_component<Health>(
                    e, 100);

                entities.push_back(e);
            }

            measure("Forge", [&] {
                for (auto e : entities) {
                    world.destroy(e);
                }
            });
        }

        {
            entt::registry world;

            std::vector<entt::entity> entities;
            entities.reserve(_config.initial_entities);

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
                auto e = world.create();

                world.emplace<Position>(
                    e, 1.0f, 2.0f);

                world.emplace<Velocity>(
                    e, 1.0f, 2.0f);

                world.emplace<Health>(
                    e, 100);

                entities.push_back(e);
            }

            measure("EnTT", [&] {
                for (auto e : entities) {
                    world.destroy(e);
                }
            });
        }
    }
    void recycling() {
        std::cout
            << "\n=== Entity Recycling ===\n";

        {
            forge::registry<
                Position,
                Health>
                world;

            std::vector<forge::entity> entities;
            entities.reserve(_config.initial_entities);

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
                auto e = world.make();

                world.add_component<Position>(
                    e,
                    static_cast<float>(i),
                    0.0f);

                world.add_component<Health>(
                    e,
                    100);

                entities.push_back(e);
            }

            measure("Forge", [&] {
                for (auto e : entities)
                    world.destroy(e);

                for (std::size_t i = 0;
                     i < _config.initial_entities;
                     ++i) {
                    auto e = world.make();

                    world.add_component<Position>(
                        e,
                        static_cast<float>(i),
                        0.0f);

                    world.add_component<Health>(
                        e,
                        100);
                }
            });
        }

        {
            entt::registry world;

            std::vector<entt::entity> entities;
            entities.reserve(_config.initial_entities);

            for (std::size_t i = 0;
                 i < _config.initial_entities;
                 ++i) {
                auto e = world.create();

                world.emplace<Position>(
                    e,
                    static_cast<float>(i),
                    0.0f);

                world.emplace<Health>(
                    e,
                    100);

                entities.push_back(e);
            }

            measure("EnTT", [&] {
                for (auto e : entities)
                    world.destroy(e);

                for (std::size_t i = 0;
                     i < _config.initial_entities;
                     ++i) {
                    auto e = world.create();

                    world.emplace<Position>(
                        e,
                        static_cast<float>(i),
                        0.0f);

                    world.emplace<Health>(
                        e,
                        100);
                }
            });
        }
    }
};
}  // namespace benchmark
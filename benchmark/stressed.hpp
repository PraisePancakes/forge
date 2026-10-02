#pragma once

#include <cstddef>
#include <cstdint>
#include <entt/entt.hpp>
#include <forge/forge.hpp>
#include <iostream>
#include <string_view>
#include <vector>

#include "benchmark.hpp"
#include "components.hpp"

namespace benchmark {

class stress_benchmark : public benchmark {
   public:
    enum class scenario {
        movement,
        health,
        health_read,
        spawn,
        destroy,
        spawn_destroy,
        full_loop
    };

    explicit stress_benchmark(configuration config = {})
        : _config{config} {}

    void run() override {
        print_header("Forge vs EnTT - Stress Benchmark");

        print_configuration();

        run_scenario(scenario::movement);
        run_scenario(scenario::health);
        run_scenario(scenario::health_read);
        run_scenario(scenario::spawn);
        run_scenario(scenario::destroy);
        run_scenario(scenario::spawn_destroy);
        run_scenario(scenario::full_loop);

        print_sink();
    }

   private:
    configuration _config;

    // ============================================================
    // Configuration
    // ============================================================

    void print_configuration() const {
        std::cout
            << "Initial entities: "
            << _config.initial_entities
            << '\n'
            << "Max entities: "
            << _config.max_entities
            << '\n'
            << "Frames: "
            << _config.frames
            << '\n'
            << "Spawns/frame: "
            << _config.spawns_per_frame
            << '\n'
            << "Destroys/frame: "
            << _config.destroys_per_frame
            << '\n'
            << "Delta time: "
            << _config.delta_time
            << '\n';
    }

    void run_scenario(scenario value) {
        std::cout
            << "\n============================================\n"
            << "Scenario: "
            << scenario_name(value)
            << '\n'
            << "============================================\n";

        forge(value);
        entt(value);
    }

    static std::string_view scenario_name(
        scenario value) {
        switch (value) {
            case scenario::movement:
                return "Movement";

            case scenario::health:
                return "Health";

            case scenario::health_read:
                return "Health Read";

            case scenario::spawn:
                return "Spawn";

            case scenario::destroy:
                return "Destroy";

            case scenario::spawn_destroy:
                return "Spawn + Destroy";

            case scenario::full_loop:
                return "Full Game Loop";
        }

        return "Unknown";
    }

    // ============================================================
    // Forge
    // ============================================================

    void forge(scenario scenario) {
        forge::registry<
            Position,
            Velocity,
            Health>
            world;

        std::vector<forge::entity> entities;

        entities.reserve(_config.max_entities);

        create_forge_world(
            world,
            entities);

        switch (scenario) {
            case scenario::movement:
                measure(
                    "Forge - Movement",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            movement_forge(world);
                        }
                    });
                break;

            case scenario::health:
                measure(
                    "Forge - Health",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            health_forge(world);
                        }
                    });
                break;
            case scenario::health_read:
                measure(
                    "Forge - Health Read",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            health_read_forge(world);
                        }
                    });
                break;
            case scenario::spawn:
                measure(
                    "Forge - Spawn",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_forge(
                                world,
                                entities);
                        }
                    });
                break;

            case scenario::destroy:
                measure(
                    "Forge - Destroy",
                    [&] {
                        destroy_all_forge(
                            world,
                            entities);
                    });
                break;

            case scenario::spawn_destroy:
                measure(
                    "Forge - Spawn + Destroy",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_forge(
                                world,
                                entities);

                            destroy_forge(
                                world,
                                entities);
                        }
                    });
                break;

            case scenario::full_loop:
                measure(
                    "Forge - Full Game Loop",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_forge(
                                world,
                                entities);

                            movement_forge(world);

                            health_forge(world);

                            destroy_forge(
                                world,
                                entities);
                        }
                    });
                break;
        }
    }

    // ============================================================
    // EnTT
    // ============================================================

    void entt(scenario scenario) {
        entt::registry world;

        std::vector<entt::entity> entities;

        entities.reserve(_config.max_entities);

        create_entt_world(
            world,
            entities);

        switch (scenario) {
            case scenario::movement:
                measure(
                    "EnTT - Movement",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            movement_entt(world);
                        }
                    });
                break;
            case scenario::health_read:
                measure(
                    "EnTT - Health Read",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            health_read_entt(world);
                        }
                    });
                break;
            case scenario::health:
                measure(
                    "EnTT - Health",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            health_entt(world);
                        }
                    });
                break;

            case scenario::spawn:
                measure(
                    "EnTT - Spawn",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_entt(
                                world,
                                entities);
                        }
                    });
                break;

            case scenario::destroy:
                measure(
                    "EnTT - Destroy",
                    [&] {
                        destroy_all_entt(
                            world,
                            entities);
                    });
                break;

            case scenario::spawn_destroy:
                measure(
                    "EnTT - Spawn + Destroy",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_entt(
                                world,
                                entities);

                            destroy_entt(
                                world,
                                entities);
                        }
                    });
                break;

            case scenario::full_loop:
                measure(
                    "EnTT - Full Game Loop",
                    [&] {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_entt(
                                world,
                                entities);

                            movement_entt(world);

                            health_entt(world);

                            destroy_entt(
                                world,
                                entities);
                        }
                    });
                break;
        }
    }

    // ============================================================
    // World creation - Forge
    // ============================================================

    void create_forge_world(
        forge::registry<
            Position,
            Velocity,
            Health>& world,
        std::vector<forge::entity>& entities) {
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

            entities.push_back(e);
        }
    }

    // ============================================================
    // World creation - EnTT
    // ============================================================

    void create_entt_world(
        entt::registry& world,
        std::vector<entt::entity>& entities) {
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

            entities.push_back(e);
        }
    }

    // ============================================================
    // Forge - Spawn
    // ============================================================

    void spawn_forge(
        forge::registry<
            Position,
            Velocity,
            Health>& world,
        std::vector<forge::entity>& entities) {
        for (std::size_t i = 0;
             i < _config.spawns_per_frame;
             ++i) {
            const auto e = world.make();

            world.add_component<Position>(
                e,
                0.0f,
                0.0f);

            world.add_component<Velocity>(
                e,
                1.0f,
                1.0f);

            world.add_component<Health>(
                e,
                100);

            entities.push_back(e);
        }
    }

    // ============================================================
    // EnTT - Spawn
    // ============================================================

    void spawn_entt(
        entt::registry& world,
        std::vector<entt::entity>& entities) {
        for (std::size_t i = 0;
             i < _config.spawns_per_frame;
             ++i) {
            const auto e = world.create();

            world.emplace<Position>(
                e,
                0.0f,
                0.0f);

            world.emplace<Velocity>(
                e,
                1.0f,
                1.0f);

            world.emplace<Health>(
                e,
                100);

            entities.push_back(e);
        }
    }

    // ============================================================
    // Forge - Movement
    // ============================================================

    void movement_forge(
        forge::registry<
            Position,
            Velocity,
            Health>& world) {
        auto view =
            world.view<
                Position,
                const Velocity>();

        view.each([&](
                      Position& position,
                      const Velocity& velocity) {
            position.x +=
                velocity.x *
                _config.delta_time;

            position.y +=
                velocity.y *
                _config.delta_time;

            sink += static_cast<std::uint64_t>(
                position.x);
        });
    }

    // ============================================================
    // EnTT - Movement
    // ============================================================

    void movement_entt(
        entt::registry& world) {
        auto view =
            world.view<
                Position,
                const Velocity>();

        view.each([&](
                      Position& position,
                      const Velocity& velocity) {
            position.x +=
                velocity.x *
                _config.delta_time;

            position.y +=
                velocity.y *
                _config.delta_time;

            sink += static_cast<std::uint64_t>(
                position.x);
        });
    }

    // ============================================================
    // Forge - Health
    // ============================================================

    void health_forge(
        forge::registry<
            Position,
            Velocity,
            Health>& world) {
        auto view = world.view<Health>();

        view.each([](Health& health) {
            health.value -= 1;

            sink += static_cast<std::uint64_t>(
                health.value);
        });
    }
    void health_read_forge(
        forge::registry<
            Position,
            Velocity,
            Health>& world) {
        auto view = world.view<Health>();

        view.each([](const Health& health) {
            sink += static_cast<std::uint64_t>(
                health.value);
        });
    }
    // ============================================================
    // EnTT - Health
    // ============================================================

    void health_entt(
        entt::registry& world) {
        auto view = world.view<Health>();
        view.each([](Health& health) {
            health.value -= 1;
            sink += static_cast<std::uint64_t>(
                health.value);
        });
    }
    void health_read_entt(
        entt::registry& world) {
        auto view = world.view<Health>();

        view.each([](const Health& health) {
            sink += static_cast<std::uint64_t>(
                health.value);
        });
    }
    // ============================================================
    // Forge - Destroy
    // ============================================================

    void destroy_forge(
        forge::registry<
            Position,
            Velocity,
            Health>& world,
        std::vector<forge::entity>& entities) {
        std::size_t destroyed = 0;

        for (auto it = entities.begin();
             it != entities.end() &&
             destroyed <
                 _config.destroys_per_frame;) {
            const auto e = *it;

            auto* health =
                world.try_get<Health>(e);

            if (health == nullptr) {
                it = entities.erase(it);
                continue;
            }

            if (health->value <= 0) {
                world.destroy(e);

                it = entities.erase(it);

                ++destroyed;
            } else {
                ++it;
            }
        }
    }

    // ============================================================
    // EnTT - Destroy
    // ============================================================

    void destroy_entt(
        entt::registry& world,
        std::vector<entt::entity>& entities) {
        std::size_t destroyed = 0;

        for (auto it = entities.begin();
             it != entities.end() &&
             destroyed <
                 _config.destroys_per_frame;) {
            const auto e = *it;

            if (!world.valid(e)) {
                it = entities.erase(it);
                continue;
            }

            auto& health =
                world.get<Health>(e);

            if (health.value <= 0) {
                world.destroy(e);

                it = entities.erase(it);

                ++destroyed;
            } else {
                ++it;
            }
        }
    }

    // ============================================================
    // Destroy everything - Forge
    //
    // This isolates destruction from the health system.
    // ============================================================

    void destroy_all_forge(
        forge::registry<
            Position,
            Velocity,
            Health>& world,
        std::vector<forge::entity>& entities) {
        for (const auto e : entities) {
            world.destroy(e);
        }

        entities.clear();
    }

    // ============================================================
    // Destroy everything - EnTT
    // ============================================================

    void destroy_all_entt(
        entt::registry& world,
        std::vector<entt::entity>& entities) {
        for (const auto e : entities) {
            world.destroy(e);
        }

        entities.clear();
    }
};

}  // namespace benchmark

#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <entt/entt.hpp>
#include <forge/forge.hpp>
#include <iomanip>
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

        // run_scenario(scenario::movement);
        // run_scenario(scenario::health);
        // run_scenario(scenario::health_read);
        // run_scenario(scenario::spawn);
        // run_scenario(scenario::destroy);
        // run_scenario(scenario::spawn_destroy);
        run_scenario(scenario::full_loop);

        print_sink();
    }

   private:
    configuration _config;

    static constexpr std::size_t warmup_runs = 3;
    static constexpr std::size_t measured_runs = 7;

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
            << '\n'
            << "Warmup runs: "
            << warmup_runs
            << '\n'
            << "Measured runs: "
            << measured_runs
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
    // Measurement
    // ============================================================

    template <typename Setup, typename Func>
    void measure_median(
        std::string_view name,
        Setup&& setup,
        Func&& func) {
        // --------------------------------------------------------
        // Warmup
        // --------------------------------------------------------

        for (std::size_t i = 0;
             i < warmup_runs;
             ++i) {
            auto world = setup();
            func(world);
        }

        // --------------------------------------------------------
        // Measured runs
        // --------------------------------------------------------

        std::vector<double> times;
        times.reserve(measured_runs);

        for (std::size_t i = 0;
             i < measured_runs;
             ++i) {
            auto world = setup();

            const auto start =
                std::chrono::steady_clock::now();

            func(world);

            const auto end =
                std::chrono::steady_clock::now();

            const double elapsed =
                std::chrono::duration<double, std::milli>(
                    end - start)
                    .count();

            times.push_back(elapsed);
        }

        std::sort(
            times.begin(),
            times.end());

        const double median =
            times[times.size() / 2];

        const double minimum =
            times.front();

        const double maximum =
            times.back();

        std::cout
            << std::left
            << std::setw(42)
            << name
            << std::right
            << std::fixed
            << std::setprecision(3)
            << median
            << " ms"
            << "  [min "
            << minimum
            << ", max "
            << maximum
            << "]\n";
    }

    // ============================================================
    // Forge
    // ============================================================

    void forge(scenario scenario) {
        switch (scenario) {
            case scenario::movement:
                measure_median(
                    "Forge - Movement",
                    [&] {
                        return std::make_unique<
                            forge_world>(_config);
                    },
                    [&](std::unique_ptr<forge_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            movement_forge(world->registry);
                        }
                    });
                break;

            case scenario::health:
                measure_median(
                    "Forge - Health",
                    [&] {
                        return std::make_unique<
                            forge_world>(_config);
                    },
                    [&](std::unique_ptr<forge_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            health_forge(world->registry);
                        }
                    });
                break;

            case scenario::health_read:
                measure_median(
                    "Forge - Health Read",
                    [&] {
                        return std::make_unique<
                            forge_world>(_config);
                    },
                    [&](std::unique_ptr<forge_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            health_read_forge(
                                world->registry);
                        }
                    });
                break;

            case scenario::spawn:
                measure_median(
                    "Forge - Spawn",
                    [&] {
                        return std::make_unique<
                            forge_world>(_config);
                    },
                    [&](std::unique_ptr<forge_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_forge(
                                world->registry,
                                world->entities);
                        }
                    });
                break;

            case scenario::destroy:
                measure_median(
                    "Forge - Destroy",
                    [&] {
                        return std::make_unique<
                            forge_world>(_config);
                    },
                    [&](std::unique_ptr<forge_world>& world) {
                        destroy_all_forge(
                            world->registry,
                            world->entities);
                    });
                break;

            case scenario::spawn_destroy:
                measure_median(
                    "Forge - Spawn + Destroy",
                    [&] {
                        return std::make_unique<
                            forge_world>(_config);
                    },
                    [&](std::unique_ptr<forge_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_forge(
                                world->registry,
                                world->entities);

                            destroy_forge(
                                world->registry,
                                world->entities);
                        }
                    });
                break;

            case scenario::full_loop:
                measure_median(
                    "Forge - Full Game Loop",
                    [&] {
                        return std::make_unique<
                            forge_world>(_config);
                    },
                    [&](std::unique_ptr<forge_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_forge(
                                world->registry,
                                world->entities);

                            movement_forge(
                                world->registry);

                            health_forge(
                                world->registry);

                            destroy_forge(
                                world->registry,
                                world->entities);
                        }
                    });
                break;
        }
    }

    // ============================================================
    // EnTT
    // ============================================================

    void entt(scenario scenario) {
        switch (scenario) {
            case scenario::movement:
                measure_median(
                    "EnTT - Movement",
                    [&] {
                        return std::make_unique<
                            entt_world>(_config);
                    },
                    [&](std::unique_ptr<entt_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            movement_entt(world->registry);
                        }
                    });
                break;

            case scenario::health:
                measure_median(
                    "EnTT - Health",
                    [&] {
                        return std::make_unique<
                            entt_world>(_config);
                    },
                    [&](std::unique_ptr<entt_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            health_entt(world->registry);
                        }
                    });
                break;

            case scenario::health_read:
                measure_median(
                    "EnTT - Health Read",
                    [&] {
                        return std::make_unique<
                            entt_world>(_config);
                    },
                    [&](std::unique_ptr<entt_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            health_read_entt(
                                world->registry);
                        }
                    });
                break;

            case scenario::spawn:
                measure_median(
                    "EnTT - Spawn",
                    [&] {
                        return std::make_unique<
                            entt_world>(_config);
                    },
                    [&](std::unique_ptr<entt_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_entt(
                                world->registry,
                                world->entities);
                        }
                    });
                break;

            case scenario::destroy:
                measure_median(
                    "EnTT - Destroy",
                    [&] {
                        return std::make_unique<
                            entt_world>(_config);
                    },
                    [&](std::unique_ptr<entt_world>& world) {
                        destroy_all_entt(
                            world->registry,
                            world->entities);
                    });
                break;

            case scenario::spawn_destroy:
                measure_median(
                    "EnTT - Spawn + Destroy",
                    [&] {
                        return std::make_unique<
                            entt_world>(_config);
                    },
                    [&](std::unique_ptr<entt_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_entt(
                                world->registry,
                                world->entities);

                            destroy_entt(
                                world->registry,
                                world->entities);
                        }
                    });
                break;

            case scenario::full_loop:
                measure_median(
                    "EnTT - Full Game Loop",
                    [&] {
                        return std::make_unique<
                            entt_world>(_config);
                    },
                    [&](std::unique_ptr<entt_world>& world) {
                        for (std::size_t frame = 0;
                             frame < _config.frames;
                             ++frame) {
                            spawn_entt(
                                world->registry,
                                world->entities);

                            movement_entt(
                                world->registry);

                            health_entt(
                                world->registry);

                            destroy_entt(
                                world->registry,
                                world->entities);
                        }
                    });
                break;
        }
    }

    // ============================================================
    // Benchmark world wrappers
    // ============================================================

    struct forge_world {
        forge::registry<
            Position,
            Velocity,
            Health>
            registry;

        std::vector<forge::entity> entities;

        explicit forge_world(
            const configuration& config) {
            entities.reserve(config.max_entities);

            create_forge_world(
                registry,
                entities,
                config);
        }
    };

    struct entt_world {
        entt::registry registry;

        std::vector<entt::entity> entities;

        explicit entt_world(
            const configuration& config) {
            entities.reserve(config.max_entities);

            create_entt_world(
                registry,
                entities,
                config);
        }
    };

    // ============================================================
    // World creation - Forge
    // ============================================================

    static void create_forge_world(
        forge::registry<
            Position,
            Velocity,
            Health>& world,
        std::vector<forge::entity>& entities,
        const configuration& config) {
        for (std::size_t i = 0;
             i < config.initial_entities;
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

    static void create_entt_world(
        entt::registry& world,
        std::vector<entt::entity>& entities,
        const configuration& config) {
        for (std::size_t i = 0;
             i < config.initial_entities;
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

        for (auto it = entities.begin(); it != entities.end() && destroyed < _config.destroys_per_frame;) {
            const auto e = *it;
            if (!world.is_alive(e)) {
                it = entities.erase(it);
                continue;
            }
            auto& health = world.get_component<Health>(e);
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
    // EnTT - Destroy
    // ============================================================

    void destroy_entt(
        entt::registry& world,
        std::vector<entt::entity>& entities) {
        std::size_t destroyed = 0;

        for (auto it = entities.begin();
             it != entities.end() && destroyed < _config.destroys_per_frame;) {
            const auto e = *it;
            if (!world.valid(e)) {
                it = entities.erase(it);
                continue;
            }
            auto& health = world.get<Health>(e);
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

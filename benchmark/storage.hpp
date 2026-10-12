#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <entt/entt.hpp>
#include <forge/forge.hpp>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

#include "benchmark.hpp"
#include "components.hpp"

namespace benchmark {

class storage_benchmark : public benchmark {
    using clock = std::chrono::steady_clock;
    using forge_storage = forge::storage::pool_storage<forge::entity, Position>;

    using entt_storage = entt::storage<Position>;

    static constexpr std::size_t entity_count = 1'000'000;
    static constexpr std::size_t warmup_runs = 3;
    static constexpr std::size_t measured_runs = 7;

    inline static volatile std::uint64_t sink = 0;

    struct forge_world {
        forge::registry<Position> registry;
        forge_storage storage;
        std::vector<forge::entity> entities;

        explicit forge_world(std::size_t count) {
            entities.reserve(count);
            for (std::size_t i = 0; i < count; ++i) entities.push_back(registry.make());
        }

        void populate() {
            storage.reserve(entities.size());

            for (std::size_t i = 0; i < entities.size(); ++i) storage.emplace(entities[i], static_cast<float>(i), static_cast<float>(i));
        }
    };

    struct entt_world {
        entt::registry registry;
        entt_storage storage;
        std::vector<entt::entity> entities;

        explicit entt_world(std::size_t count) {
            entities.reserve(count);
            for (std::size_t i = 0; i < count; ++i) entities.push_back(registry.create());
        }
        void populate() {
            storage.reserve(entities.size());
            for (std::size_t i = 0; i < entities.size(); ++i) storage.emplace(entities[i], static_cast<float>(i), static_cast<float>(i));
        }
    };

    template <typename Setup, typename Operation>
    static double measure(Setup&& setup, Operation&& operation) {
        for (std::size_t i = 0; i < warmup_runs; ++i) {
            auto world = setup();
            operation(*world);
        }

        std::vector<double> times;
        times.reserve(measured_runs);

        for (std::size_t i = 0; i < measured_runs; ++i) {
            auto world = setup();
            const auto start = clock::now();
            operation(*world);
            const auto end = clock::now();
            times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }

        std::sort(times.begin(), times.end());

        return times[times.size() / 2];
    }

    template <typename Setup, typename Prepare, typename Operation>
    static double measure_prepared(Setup&& setup, Prepare&& prepare, Operation&& operation) {
        for (std::size_t i = 0; i < warmup_runs; ++i) {
            auto world = setup();
            prepare(*world);
            operation(*world);
        }

        std::vector<double> times;
        times.reserve(measured_runs);

        for (std::size_t i = 0; i < measured_runs; ++i) {
            auto world = setup();
            prepare(*world);
            const auto start = clock::now();
            operation(*world);
            const auto end = clock::now();
            times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }

        std::sort(times.begin(), times.end());
        return times[times.size() / 2];
    }

    static void print_result(
        std::string_view operation,
        double forge_ms,
        double entt_ms) {
        std::cout << std::left << std::setw(24) << operation
                  << std::right << std::fixed
                  << std::setprecision(3)
                  << std::setw(12) << forge_ms
                  << std::setw(12) << entt_ms
                  << std::setw(12)
                  << (forge_ms > 0.0 ? entt_ms / forge_ms : 0.0)
                  << "x\n";
    }

    static void print_header() {
        std::cout << '\n'
                  << std::left << std::setw(24) << "Operation"
                  << std::right << std::setw(12) << "Forge (ms)"
                  << std::setw(12) << "EnTT (ms)"
                  << std::setw(13) << "EnTT/Forge"
                  << '\n'
                  << std::string(61, '-') << '\n';
    }

    // ------------------------------------------------------------
    // Emplace
    // ------------------------------------------------------------

    static void benchmark_emplace() {
        const auto forge_ms = measure(
            [] {
                return std::make_unique<forge_world>(entity_count);
            },
            [](forge_world& world) {
                world.storage.reserve(entity_count);

                for (std::size_t i = 0; i < entity_count; ++i) {
                    world.storage.emplace(
                        world.entities[i],
                        static_cast<float>(i),
                        static_cast<float>(i));
                }
            });

        const auto entt_ms = measure(
            [] {
                return std::make_unique<entt_world>(entity_count);
            },
            [](entt_world& world) {
                world.storage.reserve(entity_count);

                for (std::size_t i = 0; i < entity_count; ++i) {
                    world.storage.emplace(
                        world.entities[i],
                        static_cast<float>(i),
                        static_cast<float>(i));
                }
            });

        print_result("Emplace", forge_ms, entt_ms);
    }

    // ------------------------------------------------------------
    // Get
    // ------------------------------------------------------------

    static void benchmark_get() {
        const auto forge_ms = measure_prepared(
            [] {
                return std::make_unique<forge_world>(entity_count);
            },
            [](forge_world& world) {
                world.populate();
            },
            [](forge_world& world) {
                std::uint64_t sum = 0;
                for (const auto e : world.entities) {
                    sum += static_cast<std::uint64_t>(
                        world.storage.get(e).x);
                }
                sink = sum;
            });

        const auto entt_ms = measure_prepared(
            [] { return std::make_unique<entt_world>(entity_count); },
            [](entt_world& world) { world.populate(); },
            [](entt_world& world) {
                std::uint64_t sum = 0;
                for (const auto e : world.entities) sum += static_cast<std::uint64_t>(world.storage.get(e).x);
                sink = sum;
            });

        print_result("Get", forge_ms, entt_ms);
    }

    // ------------------------------------------------------------
    // Contains
    // ------------------------------------------------------------

    static void benchmark_contains() {
        const auto forge_ms = measure_prepared(
            [] { return std::make_unique<forge_world>(entity_count); },
            [](forge_world& world) { world.populate(); },
            [](forge_world& world) {
                std::uint64_t count = 0;
                for (const auto e : world.entities) count += world.storage.contains(e);
                sink = count;
            });

        const auto entt_ms = measure_prepared(
            [] { return std::make_unique<entt_world>(entity_count); },
            [](entt_world& world) { world.populate(); },
            [](entt_world& world) {
                std::uint64_t count = 0;
                for (const auto e : world.entities) count += world.storage.contains(e);
                sink = count;
            });

        print_result("Contains", forge_ms, entt_ms);
    }

    // ------------------------------------------------------------
    // Replace
    // ------------------------------------------------------------

    static void benchmark_replace() {
        const auto forge_ms = measure_prepared(
            [] { return std::make_unique<forge_world>(entity_count); },
            [](forge_world& world) { world.populate(); },
            [](forge_world& world) {
                for (std::size_t i = 0; i < entity_count; ++i) {
                    world.storage.replace(
                        world.entities[i],
                        static_cast<float>(i + 1),
                        static_cast<float>(i + 1));
                }
                sink = static_cast<std::uint64_t>(world.storage.get(world.entities.back()).x);
            });

        const auto entt_ms = measure_prepared(
            [] { return std::make_unique<entt_world>(entity_count); },
            [](entt_world& world) { world.populate(); },
            [](entt_world& world) {
                for (std::size_t i = 0; i < entity_count; ++i) {
                    world.storage.emplace(
                        world.entities[i],
                        static_cast<float>(i + 1),
                        static_cast<float>(i + 1));
                }
                sink = static_cast<std::uint64_t>(world.storage.get(world.entities.back()).x);
            });

        print_result("Replace/reconstruct", forge_ms, entt_ms);
    }

    // ------------------------------------------------------------
    // Sequential component access
    // ------------------------------------------------------------

    static void benchmark_sequential_access() {
        const auto forge_ms = measure_prepared(
            [] { return std::make_unique<forge_world>(entity_count); },
            [](forge_world& world) { world.populate(); },
            [](forge_world& world) {
                std::uint64_t sum = 0;
                for (const auto e : world.entities) sum += static_cast<std::uint64_t>(world.storage.get(e).x);
                sink = sum;
            });

        const auto entt_ms = measure_prepared(
            [] { return std::make_unique<entt_world>(entity_count); },
            [](entt_world& world) { world.populate(); },
            [](entt_world& world) {
                std::uint64_t sum = 0;
                for (const auto e : world.entities) sum += static_cast<std::uint64_t>(world.storage.get(e).x);
                sink = sum;
            });

        print_result("Sequential access", forge_ms, entt_ms);
    }

    // ------------------------------------------------------------
    // Remove
    // ------------------------------------------------------------

    static void benchmark_remove() {
        const auto forge_ms = measure_prepared(
            [] { return std::make_unique<forge_world>(entity_count); },
            [](forge_world& world) { world.populate(); },
            [](forge_world& world) {
                for (const auto e : world.entities) world.storage.remove(e);
                sink = world.storage.size();
            });

        const auto entt_ms = measure_prepared(
            [] { return std::make_unique<entt_world>(entity_count); },
            [](entt_world& world) { world.populate(); },
            [](entt_world& world) {
                for (const auto e : world.entities) world.storage.remove(e);
                sink = world.storage.size();
            });

        print_result("Remove all", forge_ms, entt_ms);
    }

   public:
    void run() override {
        std::cout
            << "Forge pool_storage vs EnTT storage\n"
            << "Entities: " << entity_count << '\n'
            << "Warmup runs: " << warmup_runs << '\n'
            << "Measured runs: " << measured_runs << '\n'
            << "Times are median milliseconds\n";

        print_header();
        benchmark_emplace();
        benchmark_get();
        benchmark_contains();
        benchmark_replace();
        benchmark_sequential_access();
        benchmark_remove();
        std::cout << "\nSink: " << sink << '\n';
    }
};

}  // namespace benchmark

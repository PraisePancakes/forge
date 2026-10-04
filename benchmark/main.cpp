#include <benchmark/benchmark.h>

#include <cstdint>
#include <forge/forge.hpp>
#include <random>
#include <unordered_map>
#include <vector>

using entity = std::uint64_t;

struct Position {
    float x;
    float y;
};

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

static std::vector<entity> make_entities(std::size_t count) {
    std::vector<entity> entities;
    entities.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        entities.push_back(
            forge::entity_traits<entity>::construct(
                static_cast<forge::entity_traits<entity>::entity_type>(i),
                0));
    }

    return entities;
}

// ------------------------------------------------------------
// Forge sparse set
// ------------------------------------------------------------

using forge_storage = forge::storage::pool_storage<entity, Position>;

// ------------------------------------------------------------
// std::unordered_map
// ------------------------------------------------------------

using map_storage = std::unordered_map<entity, Position>;

// ------------------------------------------------------------
// Insert / emplace
// ------------------------------------------------------------

static void BM_Forge_Emplace(benchmark::State& state) {
    const auto entities = make_entities(state.range(0));

    for (auto _ : state) {
        forge_storage storage;

        for (auto e : entities) {
            storage.emplace(e, 1.0f, 2.0f);
        }

        benchmark::DoNotOptimize(storage);
    }

    state.SetItemsProcessed(
        state.iterations() * state.range(0));
}

static void BM_UnorderedMap_Emplace(benchmark::State& state) {
    const auto entities = make_entities(state.range(0));

    for (auto _ : state) {
        map_storage storage;
        storage.reserve(entities.size());

        for (auto e : entities) {
            storage.emplace(e, Position{1.0f, 2.0f});
        }

        benchmark::DoNotOptimize(storage);
    }

    state.SetItemsProcessed(
        state.iterations() * state.range(0));
}

// ------------------------------------------------------------
// Lookup
// ------------------------------------------------------------

static void BM_Forge_Lookup(benchmark::State& state) {
    const auto entities = make_entities(state.range(0));

    forge_storage storage;

    for (auto e : entities) {
        storage.emplace(e, 1.0f, 2.0f);
    }

    for (auto _ : state) {
        for (auto e : entities) {
            auto& position = storage.get(e);
            benchmark::DoNotOptimize(position);
        }
    }

    state.SetItemsProcessed(
        state.iterations() * state.range(0));
}

static void BM_UnorderedMap_Lookup(benchmark::State& state) {
    const auto entities = make_entities(state.range(0));

    map_storage storage;
    storage.reserve(entities.size());

    for (auto e : entities) {
        storage.emplace(e, Position{1.0f, 2.0f});
    }

    for (auto _ : state) {
        for (auto e : entities) {
            auto& position = storage.at(e);
            benchmark::DoNotOptimize(position);
        }
    }

    state.SetItemsProcessed(
        state.iterations() * state.range(0));
}

// ------------------------------------------------------------
// Contains
// ------------------------------------------------------------

static void BM_Forge_Contains(benchmark::State& state) {
    const auto entities = make_entities(state.range(0));

    forge_storage storage;

    for (auto e : entities) {
        storage.emplace(e, 1.0f, 2.0f);
    }

    for (auto _ : state) {
        for (auto e : entities) {
            benchmark::DoNotOptimize(storage.contains(e));
        }
    }

    state.SetItemsProcessed(
        state.iterations() * state.range(0));
}

static void BM_UnorderedMap_Contains(benchmark::State& state) {
    const auto entities = make_entities(state.range(0));

    map_storage storage;
    storage.reserve(entities.size());

    for (auto e : entities) {
        storage.emplace(e, Position{1.0f, 2.0f});
    }

    for (auto _ : state) {
        for (auto e : entities) {
            benchmark::DoNotOptimize(
                storage.find(e) != storage.end());
        }
    }

    state.SetItemsProcessed(
        state.iterations() * state.range(0));
}

// ------------------------------------------------------------
// Iteration
// ------------------------------------------------------------

static void BM_Forge_Iteration(benchmark::State& state) {
    const auto entities = make_entities(state.range(0));

    forge_storage storage;

    for (auto e : entities) {
        storage.emplace(e, 1.0f, 2.0f);
    }

    for (auto _ : state) {
        for (auto& position : storage) {
            benchmark::DoNotOptimize(position);
        }
    }

    state.SetItemsProcessed(
        state.iterations() * state.range(0));
}

static void BM_UnorderedMap_Iteration(benchmark::State& state) {
    const auto entities = make_entities(state.range(0));

    map_storage storage;
    storage.reserve(entities.size());

    for (auto e : entities) {
        storage.emplace(e, Position{1.0f, 2.0f});
    }

    for (auto _ : state) {
        for (auto& [e, position] : storage) {
            benchmark::DoNotOptimize(e);
            benchmark::DoNotOptimize(position);
        }
    }

    state.SetItemsProcessed(
        state.iterations() * state.range(0));
}

// ------------------------------------------------------------
// Erase
// ------------------------------------------------------------

static void BM_Forge_Erase(benchmark::State& state) {
    const auto entities = make_entities(state.range(0));

    for (auto _ : state) {
        forge_storage storage;

        for (auto e : entities) {
            storage.emplace(e, 1.0f, 2.0f);
        }

        for (auto e : entities) {
            storage.remove(e);
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(
        state.iterations() * state.range(0));
}

static void BM_UnorderedMap_Erase(benchmark::State& state) {
    const auto entities = make_entities(state.range(0));

    for (auto _ : state) {
        map_storage storage;
        storage.reserve(entities.size());

        for (auto e : entities) {
            storage.emplace(e, Position{1.0f, 2.0f});
        }

        for (auto e : entities) {
            storage.erase(e);
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(
        state.iterations() * state.range(0));
}

// ------------------------------------------------------------
// Registration
// ------------------------------------------------------------

#define BENCHMARK_SIZES           \
    ->RangeMultiplier(4)          \
        ->Range(1 << 10, 1 << 20) \
        ->Unit(benchmark::kMicrosecond)

BENCHMARK(BM_Forge_Emplace)
BENCHMARK_SIZES;
BENCHMARK(BM_UnorderedMap_Emplace)
BENCHMARK_SIZES;

BENCHMARK(BM_Forge_Lookup)
BENCHMARK_SIZES;
BENCHMARK(BM_UnorderedMap_Lookup)
BENCHMARK_SIZES;

BENCHMARK(BM_Forge_Contains)
BENCHMARK_SIZES;
BENCHMARK(BM_UnorderedMap_Contains)
BENCHMARK_SIZES;

BENCHMARK(BM_Forge_Iteration)
BENCHMARK_SIZES;
BENCHMARK(BM_UnorderedMap_Iteration)
BENCHMARK_SIZES;

BENCHMARK(BM_Forge_Erase)
BENCHMARK_SIZES;
BENCHMARK(BM_UnorderedMap_Erase)
BENCHMARK_SIZES;
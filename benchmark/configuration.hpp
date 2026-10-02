#pragma once
#include <iostream>

namespace benchmark {
using clock = std::chrono::steady_clock;

struct configuration {
    // ------------------------------------------------------------
    // World
    // ------------------------------------------------------------

    /**
     * Number of entities created before the benchmark starts.
     */
    std::size_t initial_entities = 1'000'000;

    /**
     * Maximum number of entities the benchmark expects to hold.
     *
     * This is useful for reserving entity containers up front.
     */
    std::size_t max_entities = 1'500'000;

    // ------------------------------------------------------------
    // Game loop
    // ------------------------------------------------------------

    /**
     * Number of simulated frames.
     */
    std::size_t frames = 10'000;

    /**
     * Simulated delta time for each frame.
     *
     * Defaults to 60 FPS.
     */
    float delta_time = 1.0f / 60.0f;

    // ------------------------------------------------------------
    // Entity lifecycle
    // ------------------------------------------------------------

    /**
     * Number of entities spawned every frame.
     */
    std::size_t spawns_per_frame = 100;

    /**
     * Number of entities destroyed every frame.
     */
    std::size_t destroys_per_frame = 100;
};
};  // namespace benchmark
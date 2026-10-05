#include <entt/entt.hpp>
#include <forge/forge.hpp>
#include <iostream>

#include "fwd.hpp"

int main() {
    benchmark::configuration relaxed_config{
        .initial_entities = 1'000'000,
    };

    benchmark::configuration stress_config{
        .initial_entities = 100'000,
        .max_entities = 200'000,
        .frames = 10'000,
        .delta_time = 1.0f / 60.0f,
        .spawns_per_frame = 100,
        .destroys_per_frame = 100};

    benchmark::relaxed_benchmark relaxed{
        relaxed_config};

    benchmark::stress_benchmark stress{
        stress_config};

    // relaxed.run();
    stress.run();

    return 0;
}

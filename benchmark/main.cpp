#include <entt/entt.hpp>
#include <forge/forge.hpp>
#include <iostream>

#include "fwd.hpp"

int main() {
    benchmark::configuration relaxed_config{
        .initial_entities = 1'000'000,
    };

    benchmark::configuration stress_config{
        .initial_entities = 10'000,
        .max_entities = 20'000,
        .frames = 1'000,
        .delta_time = 1.0f / 60.0f,
        .spawns_per_frame = 10,
        .destroys_per_frame = 10};

    benchmark::relaxed_benchmark relaxed{
        relaxed_config};

    benchmark::stress_benchmark stress{
        stress_config};

    relaxed.run();
    stress.run();

    return 0;
}

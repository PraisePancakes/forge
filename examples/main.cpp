#include <cassert>
#include <forge/forge.hpp>
#include <iostream>
#include <list>
#include <tuple>
#include <vector>

int main() {
    using namespace forge;

    registry<int, char, float> world;

    // Entity 1: int + float
    auto e1 = world.make();
    world.add_component<int, float>(e1, 5, 2.3f);

    // Entity 2: int + float
    auto e2 = world.make();
    world.add_component<int, float>(e2, 10, 4.5f);

    // Entity 3: int only -> should NOT appear in the view
    auto e3 = world.make();
    world.add_component<int>(e3, 20);

    // Entity 4: float only -> should NOT appear in the view
    auto e4 = world.make();
    world.add_component<float>(e4, 6.7f);

    auto view = world.view<const int, float>();

    int count = 0;
    float float_sum = 0.0f;

    view.each([&](const auto& i, auto& f) {
        ++count;
        float_sum += f;

        std::cout << "int: " << i
                  << ", float: " << f << '\n';

        // i = 3; // Should NOT compile: i is const int&
        f += 1.0f;
    });

    // Only e1 and e2 should be visited.
    assert(count == 2);

    // 2.3 + 4.5 + 1 + 1
    assert(float_sum == 8.8f);

    // Verify the float components were actually modified.
    assert(world.get_component<float>(e1) == 3.3f);
    assert(world.get_component<float>(e2) == 5.5f);

    // Verify the int components remain unchanged.
    assert(world.get_component<int>(e1) == 5);
    assert(world.get_component<int>(e2) == 10);
    assert(world.get_component<int>(e3) == 20);

    std::cout << "All view tests passed!\n";
    return 0;
}
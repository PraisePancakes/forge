#include <cassert>
#include <forge/forge.hpp>
#include <iostream>
#include <list>
#include <tuple>
#include <vector>

int main() {
    using namespace forge;
    registry<int, char, float> world;
    auto e = world.make();
    world.add_component<int>(e, 1);
    auto v = world.view<const int>();
    v.each([&](entity e, const int& i) {
        if (i == 1) world.add_component<char>(e, 'b');
    });


    std::cout << "All view tests passed!\n";
    return 0;
}
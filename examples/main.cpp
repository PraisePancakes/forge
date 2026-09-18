#include <cassert>
#include <forge/forge.hpp>
#include <iostream>
#include <list>
#include <tuple>
#include <vector>

int main() {
    using namespace forge;
    registry<int, char, float> world;
    for (int i = 0; i < 10; i++) {
        auto e = world.make();
        if (i % 2 == 0) {
            std::cout << e << std::endl;
            world.add_component<int, char>(e, i, 'B');
        } else
            world.add_component<float, char>(e, 1.3, 'a');
    };
    auto v = world.view<const int, const char>();
    v.each([](const int& i, const char& c) {});
    std::cout << "All view tests passed!\n";
    return 0;
}
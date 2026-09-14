#include <cassert>
#include <forge/forge.hpp>
#include <iostream>
#include <tuple>

int main() {
    forge::registry<std::tuple<int, char>> world;

    for (int i = 0; i < 5; i++) {
        auto e = world.make();
        world.add_component<int>(e, i);
    }

    auto view = world.view<int>();
    view.each([](auto& i) { i += 20; });

    auto view2 = world.view<const int>();
    view2.each([](auto& i) { std::cout << i << std::endl; });

    std::cout
        << "All tests passed!\n";

    return 0;
}
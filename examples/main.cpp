#include <forge/forge.hpp>
#include <iostream>

void foo(forge::registry<int, char>& w, forge::entity e) {
    auto [intC, charC] = w.get_component<int, char>(e);
    intC = 4;
};

int main() {
    forge::registry<std::tuple<int, char>> world;
    auto e = world.make();
    world.add_component<int>(e, 5);
    world.add_component<char>(e, 'A');
    foo(world, e);
    std::cout << world.get_component<int>(e) << std::endl;
    return 0;
}
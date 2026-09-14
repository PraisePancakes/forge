#include <forge/forge.hpp>
#include <iostream>

int main() {
    forge::registry<int, char> world;
    auto e = world.make();
    auto [intC, charC] = world.add_component<int, char>(e, 1, 'c');
    std::cout << intC << " : " << charC << " " << std::endl;
    return 0;
}
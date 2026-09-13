#include <forge/forge.hpp>
#include <iostream>

void foo(const forge::registry<int, char>& w, forge::entity e) {
    const auto c = w.get_component<int>(e);
    std::cout << c;
};

int main() {
    forge::registry<std::tuple<int, char>> world;
    auto e = world.make();
    auto component = world.add_component<int>(e, 5);
    std::cout << component << std::endl;
    return 0;
}
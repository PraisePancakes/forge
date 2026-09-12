#include <forge/forge.hpp>
#include <iostream>

int main() {
    forge::registry world;
    world.make();
    world.make();
    auto e = world.make();
    e = forge::next(e);
    e = forge::next(e);
    e = forge::next(e);
    e = forge::next(e);
    e = forge::next(e);
    std::cout << e;
    return 0;
}
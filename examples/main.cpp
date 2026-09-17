#include <cassert>
#include <forge/forge.hpp>
#include <iostream>
#include <tuple>

int main() {
    std::tuple<int, char> tup{1, 'c'};
    forge::meta::runtime_tuple_get(0, tup, [](const int arg) { return arg; });

    return 0;
}
#include <cassert>
#include <forge/forge.hpp>
#include <iostream>
#include <list>
#include <tuple>
#include <vector>

int main() {
    using namespace forge::meta::_INTERNAL;
    std::tuple<std::vector<int>, std::vector<char>> t({1, 2}, {'x'});
    std::size_t i = 1;
    homogeneous_template_tuple_get(i, t,
                                   [](auto& x) {
                                       std::cout << x.size() << '\n';
                                   });
    return 0;
}
#include <forge/forge.hpp>
#include <iostream>

int main() {
    static_assert(forge::meta::is_unique_set<int, char>::value);
    static_assert(std::is_same_v<forge::meta::type_of<2, int, char, bool>::type, bool>);
    return 0;
}
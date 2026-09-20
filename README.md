# Forge
Forging a world one entity at a time. `Forge` is a header-only, lightweight and static entity-component system written in **modern C++**.

* [Introduction](#introduction)
    * [Motivation](#motivation)
    * [Example](#example)
* [Usage](#usage)
* [Contributing](#contributions)
   

# Introduction
An entity-component-system (ECS) is a reputable architectural pattern used by many game developers. This [pattern](https://github.com/SanderMertens/ecs-faq) is famous for it's composability of entities and intuitive design, these architectural attributes make extending current projects much easier to approach.
## Motivation
This project is heavily inspired by EnTT. I have a great deal of respect for [skypjack](https://github.com/skypjack), its creator, whose work has been a major source of inspiration for me—and, I hope, for many other developers as well.
If you’re interested in learning more, be sure to check out `EnTT` [here](https://github.com/skypjack/entt), an excellent and influential C++ entity-component system.


## Example

```cpp
#include <forge/forge.hpp>

int main() {
    // define your registry with a list of components
    forge::registry<int, char, float, std::string, long> world;

    // make an entity
    forge::entity e = world.make();

    // compose singularly
    world.add_component<int>(e, 12);

    // compose concurrently
    world.add_component<char, float>(e, 'a', 1.2);

    // get singularly (immutable)
    // world.get_component<const int>(e) = 14; fails
    // get singularly (mutable)
    world.get_component<int>(e) = 14;  // component is 14

    // get concurrently (mutable) -> tuple of mutable references
    auto [i, c] = world.get_component<int, char>(e);
    i = 4;

    std::cout << "=== Get concurrently (immutable) -> tuple of immutable references ===" << std::endl;
    auto [i2, c2] = world.get_component<const int, const char>(e);
    std::cout << "i2 : " << i2 << ", c2 : " << c2 << std::endl;  // i2 : 4, c2 : a

    std::cout << "=== Implicit has all ===" << std::endl;
    std::cout << std::boolalpha << world.has_component<int, char>(e) << std::endl;         // true
    std::cout << std::boolalpha << world.has_component<std::string, int>(e) << std::endl;  // false

    std::cout << "=== Explicit has all ===" << std::endl;
    std::cout << std::boolalpha << world.has_component<std::logical_and, int, char>(e) << std::endl;         // true
    std::cout << std::boolalpha << world.has_component<std::logical_and, std::string, int>(e) << std::endl;  // false

    std::cout << "=== Explicit has or ===" << std::endl;
    std::cout << std::boolalpha << world.has_component<std::logical_or, std::string, int>(e) << std::endl;   // true
    std::cout << std::boolalpha << world.has_component<std::logical_or, std::string, long>(e) << std::endl;  // false

    // destroy
    std::cout << "=== Before destroy===" << std::endl;
    std::cout << e << std::endl;
    std::cout << "is alive : " << std::boolalpha << world.is_alive(e) << std::endl;  // true
    world.destroy(e);
    std::cout << "=== After destroy===" << std::endl;
    std::cout << "is alive : " << std::boolalpha << world.is_alive(e) << std::endl;  // false

    for (int i = 0; i < 10; i++) {
        auto e = world.make();
        std::cout << "making entity " << e << std::endl;
        if (i % 2 == 0)
            world.add_component<int, std::string>(e, i, "even");
        else
            world.add_component<int, char>(e, i, 'O');
    }

    std::cout << "=== Make a immutable view ===" << std::endl;
    auto immutable_view = world.view<const int, const std::string>();
    immutable_view.each([](auto& i, auto& s) {
        // s = "test"; fails
        // i = 2; fails
        std::cout << "Int component : " << i << ", String component : " << s << std::endl;
    });

    std::cout << "=== Make an extendable view ===" << std::endl;
    immutable_view.each([](const forge::entity e, auto& i, auto& s) {
        std::cout << e << " has Int component: " << i << ", String component: " << s << std::endl;
    });

    std::cout << "=== Make a mutable view ===" << std::endl;
    auto mutable_view = world.view<int, std::string>();
    mutable_view.each([&world](const forge::entity e, auto& i, auto& s) {
        if (forge::to_id(e) == 0) {
            s = "Not even";
            std::cout << e << " Int component: " << i << " String component: " << world.get_component<std::string>(e) << std::endl;
        }
    });

    std::cout << "=== View with iterator ===" << std::endl;
    for (const auto e : mutable_view) {
        std::cout << e << std::endl;
    }

    std::cout << "===  Each with Mutable iterator ===" << std::endl;
    for (auto [i, s] : mutable_view.each()) {
        i = 4;
    };

    mutable_view.each([&world](const forge::entity e, auto& i, auto& s) {
        std::cout << e << " Int component: " << i << " String component: " << s << std::endl;
    });
    return 0;
}

```
**OUTPUT:**
```
=== Get concurrently (immutable) -> tuple of immutable references ===
i2 : 4, c2 : a
=== Implicit has all ===
true
false
=== Explicit has all ===
true
false
=== Explicit has or ===
true
false
=== Before destroy===
Entity { ID : 0, VERSION : 0}
is alive : true
=== After destroy===
is alive : false
making entity Entity { ID : 0, VERSION : 1}
making entity Entity { ID : 1, VERSION : 0}
making entity Entity { ID : 2, VERSION : 0}
making entity Entity { ID : 3, VERSION : 0}
making entity Entity { ID : 4, VERSION : 0}
making entity Entity { ID : 5, VERSION : 0}
making entity Entity { ID : 6, VERSION : 0}
making entity Entity { ID : 7, VERSION : 0}
making entity Entity { ID : 8, VERSION : 0}
making entity Entity { ID : 9, VERSION : 0}
=== Make a immutable view ===
Int component : 0, String component : even
Int component : 2, String component : even
Int component : 4, String component : even
Int component : 6, String component : even
Int component : 8, String component : even
=== Make an extendable view ===
Entity { ID : 0, VERSION : 1} has Int component: 0, String component: even
Entity { ID : 2, VERSION : 0} has Int component: 2, String component: even
Entity { ID : 4, VERSION : 0} has Int component: 4, String component: even
Entity { ID : 6, VERSION : 0} has Int component: 6, String component: even
Entity { ID : 8, VERSION : 0} has Int component: 8, String component: even
=== Make a mutable view ===
Entity { ID : 0, VERSION : 1} Int component: 0 String component: Not even
=== View with iterator ===
Entity { ID : 0, VERSION : 1}
Entity { ID : 2, VERSION : 0}
Entity { ID : 4, VERSION : 0}
Entity { ID : 6, VERSION : 0}
Entity { ID : 8, VERSION : 0}
===  Each with Mutable iterator ===
Entity { ID : 0, VERSION : 1} Int component: 4 String component: Not even
Entity { ID : 2, VERSION : 0} Int component: 4 String component: even
Entity { ID : 4, VERSION : 0} Int component: 4 String component: even
Entity { ID : 6, VERSION : 0} Int component: 4 String component: even
Entity { ID : 8, VERSION : 0} Int component: 4 String component: even
```
# Usage
`Forge` is a header-only library, simply `#include <forge/forge.hpp` at the top of your file and you got it!
## Requirements
`Forge` is built on a compiler that supports at least C++23.
Recommended Compiler Specs. include `Clang 20.1.2 x86_64` and `GCC 14.2.0 x86_64`.
This project requires `CMake` version 3.28 or later.
# Contributions
`Forge` is an open source library, contributions are not only welcomed but encouraged. Feel free to create an issue or submit a pull request from a new branch.
I will gladly review it and give my feedback.









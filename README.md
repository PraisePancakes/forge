# Forge
Forging a world one entity at a time. `Forge` is a header-only, lightweight and static entity-component system written in **modern C++**.

* [Introduction](#introduction)
    * [Motivation](#motivation)
    * [Examples](#examples)
        * [Hello&nbsp;World](#hello-world)
        * [Entities](#entities-identifiers-and-everything-in-between)
        * [Components](#components)
        * [Systems&nbsp;and&nbsp;Views](#systems-make-the-world-go-round)
        * [Signals&nbsp;and&nbsp;Events](#what-an-eventful-world)
        * [All-in-one](#all-in-one)
* [Benchmarks](#benchmarks)
* [Usage](#usage)
* [Contributing](#contributions)
   

# Introduction
An entity-component-system (ECS) is a reputable architectural pattern used by many game developers. This [pattern](https://github.com/SanderMertens/ecs-faq) is famous for it's composability of entities and intuitive design, these architectural attributes make extending current projects much easier to approach.
## Motivation
This project is heavily inspired by EnTT. I have a great deal of respect for [skypjack](https://github.com/skypjack), its creator, whose work has been a major source of inspiration for me—and, I hope, for many other developers as well.
If you’re interested in learning more, be sure to check out `EnTT` [here](https://github.com/skypjack/entt), an excellent and influential C++ entity-component system.


## Examples

# Hello World
Let us start from the very beginning. The first thing you must know about this ECS is that, unlike others, your components must be given to the registry up front.
This policy is in place to efficiently delegate most of the work required for the allocation of component pools to compilation rather than runtime, saving on runtime costs like rtti and vtables.

```cpp
#include <forge/forge.hpp>
int main() {
     forge::registry<int, char> world;
}
```
Now that we have our registry set up let's move on.

# Entities, Identifiers, and everything in between
In a standard ECS an entity is simply a number, nothing more nothing less. This numeric identifier is the foundation for all component relationships. 
In `Forge` an entity identifier is a number with a packed bit representation. Entities can either be a 64-bit unsigned integer or a 32-bit unsigned integer.
For simplicity sake let's imagine an 8-bit representation of an entity.
`0001 0010`
Here the higher 4 bits (`0001`) represent the entity's id. This id is most useful for the component relationships mentioned above. This id is the basis for all component look-ups, updates, removals, etc...
the lower 4 bits (`0010`) represent the entity's version, this entity is on it's second version, meaning it has been recycled twice. Recycling entities is important for handling storage memory efficiently (in the case of sparse storage) and ensuring that entities don't grow faster than needed by your world. Versioning also determines whether an old entity of the same id is stale or valid. To change the size of an entity's representation refer to `forge/config/entity_configuration.hpp`, there you can change the size from the defaulted `std::uint64_t` to `std::uint32_t`. So now that we have our world, let's create an entity.

```cpp
#include <forge/forge.hpp>
int main() {
     forge::registry<int, char> world;
     forge::entity e = world.make();
     // we can also get the id/version using some helpers
     auto id = forge::to_id(e);
     auto version = forge::to_version(e);
     // and similarly, though not needed, we can update the entity's version. This does NOT modify the original version.
     forge::entity next_version = forge::next(e);
     // check if alive
     bool alive = world.is_alive(e);
     // destroy the entity (removes all its components from their respective pools)
     world.destroy(e);
     // check if dead
     bool dead = !world.is_alive(e);
}
```
# Components put the C into ECS
`Forge` ensures that your components are type safe, meaning if you do not have your component registered in the registry's type-list a compilation error will be raised.
Now let's add a component:
```cpp
#include <forge/forge.hpp>
int main() {
     forge::registry<int, char> world;
     forge::entity e = world.make();
     auto component = world.add_component<int>(e, 4);
     forge::entity e2 = world.make();
     // we can also add a component concurrently
     auto [i, c] = world.add_component<int, char>(e2, 1, 'c');
}
```
let's query some components:
```cpp
#include <forge/forge.hpp>
int main() {
     forge::registry<int, char> world;
     forge::entity e = world.make();
     world.add_component<int, char>(e, 1, 'c');
     auto [i, c] = world.get_component<int, char>(e);
     std::cout << i << " : " << c << std::endl;

     auto* try_component = world.try_get<int>(e);
     if(try_component) std::cout << *try_component;
}
```
let's check for components:
```cpp
#include <forge/forge.hpp>
int main() {
    forge::registry<int, char, std::string, long> world;
    forge::entity e = world.make();
    world.add_component<int, char>(e, 1, 'c');
     
     // implicitly check logical-and
    std::cout << std::boolalpha << world.has_component<int, char>(e) << std::endl;         // true
    std::cout << std::boolalpha << world.has_component<std::string, int>(e) << std::endl;  // false

    // explicitly check logical-and
    std::cout << std::boolalpha << world.has_component<std::logical_and, int, char>(e) << std::endl;         // true
    std::cout << std::boolalpha << world.has_component<std::logical_and, std::string, int>(e) << std::endl;  // false

    // explicitly check logical-or
    std::cout << std::boolalpha << world.has_component<std::logical_or, std::string, int>(e) << std::endl;   // true
    std::cout << std::boolalpha << world.has_component<std::logical_or, std::string, long>(e) << std::endl;  // false
}
```
We can also replace a component:
```cpp
#include <forge/forge.hpp>
int main() {
 forge::registry<int, char, std::string, long> world;
 forge::entity e = world.make();
 world.add_component<int, char>(e, 1, 'c');
 world.replace_component<int>(e, 4);
 // add_or_replace
 world.add_or_replace_component<char>(e, 'b'); // replace
 world.add_or_replace_component<std::string>(e, "added"); // add
}
```
finally lets remove components:
```cpp
#include <forge/forge.hpp>
int main() {
 forge::registry<int, char, std::string, long> world;
 forge::entity e = world.make();
 world.add_component<int, char>(e, 1, 'c');
 bool removed = world.remove_component<int>(e); // true
 bool removed1 = world.remove_component<long>(e) // false
}
```
# Systems make the world go round
`Forge` can model systems using views. A view is just a projection of all the entities that are associated with a subset of components. Let's *view* some examples:
```cpp
#include <forge/forge.hpp>
int main() {
    forge::registry<int, char, std::string, long> world;
    for (int i = 0; i < 10; i++) {
            auto e = world.make();
            std::cout << "making entity " << e << std::endl;
            if (i % 2 == 0)
                world.add_component<int, std::string>(e, i, "even");
            else
                world.add_component<int, char>(e, i, 'O');
    }
    // make an immutable view
    auto immutable_view = world.view<const int, const std::string>();
    immutable_view.each([](auto& i, auto& s) {
        // s = "test"; fails
        // i = 2; fails
        std::cout << "Int component : " << i << ", String component : " << s << std::endl;
    });
    // make an extendable view
    immutable_view.each([](const forge::entity e, auto& i, auto& s) {
        std::cout << e << " has Int component: " << i << ", String component: " << s << std::endl;
    });

    //make a mutable view
    auto mutable_view = world.view<int, std::string>();
    mutable_view.each([&world](const forge::entity e, auto& i, auto& s) {
        if (forge::to_id(e) == 0) {
            s = "Not even";
            std::cout << e << " Int component: " << i << " String component: " << world.get_component<std::string>(e) << std::endl;
        }
    });

    //iterator compliance
    for (const auto e : mutable_view) {
        std::cout << e << std::endl;
    }

    for (auto [i, s] : mutable_view.each()) {
        i = 4;
    };
    mutable_view.each([](const forge::entity e, auto& i, auto& s) {
        std::cout << e << " Int component: " << i << " String component: " << s << std::endl;
    });
}
```
We can also exclude some subset of components from the view.
```cpp
#include <forge/forge.hpp>
int main() {
    forge::registry<int, char, std::string, long> world;
    auto e1 = world.make();
    world.add_component<int, char>(e1, 100, 'A');

    // Entity with int + char + string
    auto e2 = world.make();
    world.add_component<int, char, std::string>(e2, 200, 'B', "excluded");

    // Entity with int + char
    auto e3 = world.make();
    world.add_component<int, char>(e3, 300, 'C');

    // Entity with int + char + string
    auto e4 = world.make();
    world.add_component<int, char, std::string>(e4, 400, 'D', "excluded");

    // Only entities with int + char AND WITHOUT string
    auto ex_view = world.view<int, char>().exclude<std::string>();

    for (const auto e : ex_view) {
        auto [i, c] = world.get_component<int, char>(e);
        std::cout << e
                  << " -> int: " << i
                  << ", char: " << c
                  << std::endl;
    }
}
```
# What an eventful world
We can use signals to notify when an event has happened in our world.
```cpp
#include <forge/forge.hpp>

int main() {
    forge::registry<int, char, float, std::string, long> world;
     world.on_construct<int>().connect([](forge::entity e, int v) {
        std::cout << "triggered construction on entity " << e << " with int [" << v << "]" << std::endl;
    });

    world.on_destroy<std::string>().connect([](forge::entity e, std::string v) {
        std::cout << "triggered destruction on entity " << e << " with std::string [" << v << "]" << std::endl;
    });

    // NOTE on_update only works with forge::registry<Ts...>::replace_component<T> currently
    world.on_update<int>().connect([](forge::entity e, int v) {
        std::cout << "triggered update on entity " << e << " with new integer [" << v << "]" << std::endl;
    });

    forge::entity e = world.make();
    // Triggers on_construct<int>.
    world.add_component<int>(e, 42);
    // Triggers on_update<int>.
    world.replace_component<int>(e, 100);
    // Construct a string component.
    world.add_component<std::string>(e, "hello");
    // Triggers on_destroy<std::string>.
    world.remove_component<std::string>(e);
}
```
# All for one and one for all
```cpp
#include <forge/forge.hpp>

int main() {
    // define your registry with a list of components
    forge::registry<int, char, float, std::string, long> world;

    // signals
    world.on_construct<int>().connect([](forge::entity e, int v) {
        std::cout << "triggered construction on entity " << e << " with int [" << v << "]" << std::endl;
    });

    world.on_destroy<std::string>().connect([](forge::entity e, std::string v) {
        std::cout << "triggered destruction on entity " << e << " with std::string [" << v << "]" << std::endl;
    });

    // NOTE on_update only works with forge::registry<Ts...>::replace_component<T> currently
    world.on_update<int>().connect([](forge::entity e, int v) {
        std::cout << "triggered update on entity " << e << " with new integer [" << v << "]" << std::endl;
    });

    // make an entity
    forge::entity e = world.make();

    // compose singularly
    world.add_component<int>(e, 12);
    world.add_component<std::string>(e, "string");
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

    std::cout << "=== Update an existing int component from entity 0 ===" << std::endl;
    world.replace_component<int>(e, 4);
    std::cout << "=== Remove component int and string from entity 0 ===" << std::endl;
    world.remove_component<int, std::string>(e);
    std::cout << std::boolalpha << world.has_component<int>(e) << std::endl;  // false

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

    mutable_view.each([](const forge::entity e, auto& i, auto& s) {
        std::cout << e << " Int component: " << i << " String component: " << s << std::endl;
    });

    std::cout << "=== View with exclusion ===" << std::endl;

    // Entity with int + char
    auto e1 = world.make();
    world.add_component<int, char>(e1, 100, 'A');

    // Entity with int + char + string
    auto e2 = world.make();
    world.add_component<int, char, std::string>(e2, 200, 'B', "excluded");

    // Entity with int + char
    auto e3 = world.make();
    world.add_component<int, char>(e3, 300, 'C');

    // Entity with int + char + string
    auto e4 = world.make();
    world.add_component<int, char, std::string>(e4, 400, 'D', "excluded");

    // Only entities with int + char AND WITHOUT string
    auto ex_view = world.view<int, char>().exclude<std::string>();

    for (const auto e : ex_view) {
        auto [i, c] = world.get_component<int, char>(e);
        std::cout << e
                  << " -> int: " << i
                  << ", char: " << c
                  << std::endl;
    }
    return 0;
}


```
**OUTPUT:**
```
triggered construction on entity Entity { ID : 0, VERSION : 0} with int [12]
=== Get concurrently (immutable) -> tuple of immutable references ===
i2 : 4, c2 : a
=== Implicit has all ===
true
true
=== Explicit has all ===
true
true
=== Explicit has or ===
true
true
=== Update an existing int component from entity 0 ===
triggered update on entity Entity { ID : 0, VERSION : 0} with new integer [4]
=== Remove component int and string from entity 0 ===
triggered destruction on entity Entity { ID : 0, VERSION : 0} with std::string [string]
false
=== Before destroy===
Entity { ID : 0, VERSION : 0}
is alive : true
=== After destroy===
is alive : false
making entity Entity { ID : 0, VERSION : 1}
triggered construction on entity Entity { ID : 0, VERSION : 1} with int [0]
making entity Entity { ID : 1, VERSION : 0}
triggered construction on entity Entity { ID : 1, VERSION : 0} with int [1]
making entity Entity { ID : 2, VERSION : 0}
triggered construction on entity Entity { ID : 2, VERSION : 0} with int [2]
making entity Entity { ID : 3, VERSION : 0}
triggered construction on entity Entity { ID : 3, VERSION : 0} with int [3]
making entity Entity { ID : 4, VERSION : 0}
triggered construction on entity Entity { ID : 4, VERSION : 0} with int [4]
making entity Entity { ID : 5, VERSION : 0}
triggered construction on entity Entity { ID : 5, VERSION : 0} with int [5]
making entity Entity { ID : 6, VERSION : 0}
triggered construction on entity Entity { ID : 6, VERSION : 0} with int [6]
making entity Entity { ID : 7, VERSION : 0}
triggered construction on entity Entity { ID : 7, VERSION : 0} with int [7]
making entity Entity { ID : 8, VERSION : 0}
triggered construction on entity Entity { ID : 8, VERSION : 0} with int [8]
making entity Entity { ID : 9, VERSION : 0}
triggered construction on entity Entity { ID : 9, VERSION : 0} with int [9]
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
=== View with exclusion ===
triggered construction on entity Entity { ID : 10, VERSION : 0} with int [100]
triggered construction on entity Entity { ID : 11, VERSION : 0} with int [200]
triggered construction on entity Entity { ID : 12, VERSION : 0} with int [300]
triggered construction on entity Entity { ID : 13, VERSION : 0} with int [400]
Entity { ID : 1, VERSION : 0} -> int: 1, char: O
Entity { ID : 3, VERSION : 0} -> int: 3, char: O
Entity { ID : 5, VERSION : 0} -> int: 5, char: O
Entity { ID : 7, VERSION : 0} -> int: 7, char: O
Entity { ID : 9, VERSION : 0} -> int: 9, char: O
Entity { ID : 10, VERSION : 0} -> int: 100, char: A
Entity { ID : 12, VERSION : 0} -> int: 300, char: C
```
# Benchmarks
For the sake of comparison, I wrote a very basic benchmark against EnTT. 
**NOTE:** 
 This benchmark is not stressful so results may vary with added stress. Feel free to add stress tests to the benchmark.

`Benchmark Results`
```
============================================
        Forge vs EnTT Benchmark
============================================
Entities: 1000000

=== Entity Creation ===
Forge                                         32.197 ms
EnTT                                         160.382 ms

=== Entity + Component Creation ===
Forge                                        808.273 ms
EnTT                                        2134.114 ms

=== View Iteration ===
Forge                                        180.751 ms
EnTT                                         222.494 ms

=== View Iteration + Entity ===
Forge                                        171.800 ms
EnTT                                         261.693 ms

=== Const View Iteration ===
Forge                                        172.137 ms
EnTT                                         231.161 ms

=== Component Lookup ===
Forge                                         22.668 ms
EnTT                                         173.564 ms

=== Entity Destruction ===
Forge                                        297.728 ms
EnTT                                        1144.321 ms

=== Entity Recycling ===
Forge                                        332.913 ms
EnTT                                        1841.774 ms

Sink: 4999999000000
```

# Usage
`Forge` is a header-only library, simply `#include <forge/forge.hpp` at the top of your file and you got it!

## Requirements
`Forge` is built on a compiler that supports at least C++23.
Recommended Compiler Specs. include `Clang 20.1.2 x86_64` and `GCC 14.2.0 x86_64`.
This project requires `CMake` version 3.28 or later.
# Building
 You can use `Forge` from a CMake project by simpling linking an existing target to the `forge::forge` alias. The library can be fetched using `add_subdirectory()` or `FetchContent_Declare()`

 **Building Tests/Examples/Benchmarks**
 
 To build tests/examples/benchmarks navigate to the CMakeLists.txt file in the root directory of `Forge` and locate 
 `option(FORGE_BUILD_EXAMPLES "Build Forge examples" OFF)`
 `option(FORGE_BUILD_TESTS "Build Forge tests" OFF)`
 `option(FORGE_BUILD_BENCHMARK "Build Forge benchmarks" OFF)`
 you can then switch from `OFF` to `ON` which will build the options.
# Contributions
`Forge` is an open source library, contributions are not only welcomed but encouraged. Feel free to create an issue or submit a pull request from a new branch.
I will gladly review it and give my feedback.









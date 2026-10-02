#include <forge/forge.hpp>
int main() {
#if 1
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
#endif
    return 0;
}

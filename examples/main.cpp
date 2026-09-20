#include <forge/forge.hpp>

int main() {
#if 1
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
#endif
    return 0;
}

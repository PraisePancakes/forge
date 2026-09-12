#pragma once
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

#include "entity.hpp"
#include "meta.hpp"
#include "storage.hpp"
namespace forge {

template <typename... ComponentRegistry>
    requires(meta::is_unique_set_v<ComponentRegistry...>)
class registry {
   public:
    using entity = entity<std::uint64_t>;
    std::tuple<storage::sparse_set<ComponentRegistry>...> storage_map;

    entity make() {
        // dont just return we must store this somewhere too remember dat.
        return entity{};
    };

    // world.add_component<Position>(e, 1, 2);
    template <typename Component, typename... Args>
        requires((std::is_object_v<Component> || std::is_destructible_v<Component>) &&
                 meta::contains_it<Component, ComponentRegistry...>)
    Component add_component(entity e, Args&&... args) {
        Component c{std::forward<Args>(args)...};
    };
};
};  // namespace forge
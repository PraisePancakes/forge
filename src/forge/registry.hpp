#pragma once
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

#include "entity.hpp"
#include "meta.hpp"
#include "storage.hpp"
namespace forge {
using entity = entity_fwd<std::uint64_t>;

template <typename... ComponentRegistry>
    requires(meta::is_unique_set_v<ComponentRegistry...>)
class registry {
    template <typename T>
    using sparse_set_t = storage::sparse_set<T, entity::id_type>;

   public:
    std::tuple<sparse_set_t<ComponentRegistry>...> storage_map;
    /**
     * @brief creates an entity identifier.
     */
    entity make() {
        return entity{};
    };

    /**
     * @brief adds component of given type, component must be registered to the ComponentRegistry
     *
     * @tparam Component type of component to add.
     * @tparam Args variadic types of arguments
     * @param e the entity to add the component to
     * @param Args variadic list of arguments to forward to in-place component construction.
     */
    template <typename Component, typename... Args>
        requires((std::is_object_v<Component> || std::is_destructible_v<Component>) &&
                 meta::contains_it<Component, ComponentRegistry...>)
    Component& add_component(entity e, Args&&... args) {
        sparse_set_t<Component>& storage = std::get<meta::index_of<Component, ComponentRegistry...>::index>(storage_map);
        storage.emplace(to_id(e), std::forward<Args>(args)...);
        return storage.get(to_id(e));
    };
    /**
     * @brief gets component of given type, component must be registered to the ComponentRegistry
     *
     * @tparam Component type of component to retrieve.
     * @param self allows for deducing-this qualifiers to be propagated to user
     * @param e the entity to allow component retrieval from the component's storage
     */
    template <typename Component>
    decltype(auto) get_component(this auto& self, entity e) {
        auto& storage = std::get<meta::index_of<Component, ComponentRegistry...>::index>(self.storage_map);
        return storage.get(e);
    }
};
/**
 * @brief registry specialization on a tuple-like component registry
 *
 * @tparam T tuple-like component registry.
 */
template <template <typename...> class T, typename... Ts>
    requires(meta::is_unique_set_v<T<Ts...>>)
class registry<T<Ts...>> : public registry<Ts...> {};
};  // namespace forge
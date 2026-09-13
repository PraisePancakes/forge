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
     * @return reference to component object
     */
    template <typename Component>
    [[nodiscard]] decltype(auto) get_component(this auto& self, entity e) noexcept {
        constexpr static std::size_t component_index = meta::index_of<Component, ComponentRegistry...>::index;
        auto& storage = std::get<component_index>(self.storage_map);
        FORGE_ASSERT(storage.contains(to_id(e)), "[SPARSE GET ASSERTION FAILED] Key " << to_id(e) << " is not contained in the sparse storage of component index " << component_index);
        return storage.get(to_id(e));
    }
    /**
     * @brief gets components of given type, components must be registered to the ComponentRegistry
     *
     * @tparam Components type list to retrieve.
     * @param self allows for deducing-this qualifiers to be propagated to user
     * @param e the entity to allow component retrieval from the component's storage
     * @return forwarded tuple of reference to component objects
     */
    template <typename... Components>
        requires(sizeof...(Components) > 1)
    [[nodiscard]] decltype(auto) get_component(this auto& self, entity e) noexcept {
        return std::forward_as_tuple(self.template get_component<Components>(e)...);
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
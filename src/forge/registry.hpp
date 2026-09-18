#pragma once
#include <iostream>
#include <stack>
#include <tuple>
#include <type_traits>
#include <utility>

#include "entity.hpp"
#include "meta.hpp"
#include "storage.hpp"
#include "view.hpp"
namespace forge {
using entity = entity_fwd<std::uint64_t>;

template <typename... ComponentRegistry>
    requires(meta::is_unique_set_v<ComponentRegistry...> && !meta::contains_it<bool, ComponentRegistry...>)
class registry {
    template <typename T>
    using sparse_set_t = storage::sparse_set<T, entity>;
    std::stack<entity> entity_store;
    std::vector<entity::version_type> version_history;

    template <typename T>
    static constexpr std::size_t index_of_type = meta::index_of<std::remove_cvref_t<T>, ComponentRegistry...>::value;

    template <std::size_t Index>
    using type_of_t = meta::type_of_t<Index, ComponentRegistry...>;

    template <typename... Ts>
    using storage_pool_type = std::tuple<sparse_set_t<type_of_t<index_of_type<Ts>>>...>;
    std::tuple<sparse_set_t<ComponentRegistry>...> storage_map;

   public:
    registry() = default;
    registry(const registry&) = delete;
    registry& operator=(const registry&) = delete;
    registry(registry&&) = default;
    registry& operator=(registry&&) = default;

    template <typename... Ts>
        requires(sizeof...(Ts) <= sizeof...(ComponentRegistry))
    [[nodiscard]] decltype(auto) view() noexcept {
        static_assert((meta::contains_it<std::remove_cvref_t<Ts>, ComponentRegistry...> && ...), "Error: Provided view type(s) is not a subset of the world's component registry!");

        return view_span<entity, Ts...>(
            std::forward_as_tuple(
                std::get<index_of_type<std::remove_cvref_t<Ts>>>(storage_map)...));
    }
    /**
     * @brief creates a new entity identifier unless an identifier can be recycled then we use the next version of the recycled identifier.
     */
    entity make() {
        if (!entity_store.empty()) {
            auto e = entity_store.top();
            entity_store.pop();
            return next(e);
        }
        entity e{};
        version_history.push_back(to_version(e));
        return e;
    };

    [[nodiscard]] bool is_alive(const entity e) const noexcept {
        return version_history[to_id(e)] == to_version(e);
    }

    template <typename C>
    [[nodiscard]] bool has_component(const entity e) const noexcept {
        using stripped_type = std::remove_cvref_t<C>;
        const sparse_set_t<stripped_type>& storage = std::get<meta::index_of<stripped_type, ComponentRegistry...>::value>(storage_map);
        return storage.contains(e);
    };

    template <template <typename> typename Logical, typename... Cs>
        requires((sizeof...(Cs) > 1) && (std::is_same_v<std::logical_and<>, Logical<void>> || std::is_same_v<std::logical_or<>, Logical<void>>))
    [[nodiscard]] bool has_component(const entity e) const noexcept {
        if constexpr (std::is_same_v<Logical<void>, std::logical_and<>>)
            return (has_component<Cs>(e) && ...);
        else
            return (has_component<Cs>(e) || ...);
    }

    template <typename... Cs>
        requires(sizeof...(Cs) > 1)
    [[nodiscard]] bool has_component(const entity e) const noexcept {
        return has_component<std::logical_and, Cs...>(e);
    }

    // destroy the entity and remove all its components from the sparse set storage
    void destroy(const entity e) {
        if (!is_alive(e)) return;
        std::apply([&e](auto&&... sets) { (sets.remove(e), ...); }, this->storage_map);
        version_history[to_id(e)] = to_version(next(e));
        this->entity_store.push(e);
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
    Component& add_component(const entity e, Args&&... args) {
        sparse_set_t<Component>& storage = std::get<meta::index_of<Component, ComponentRegistry...>::value>(storage_map);
        storage.emplace(e, std::forward<Args>(args)...);
        return storage.get(e);
    };

    template <typename... Components, typename... Args>
        requires(sizeof...(Components) == sizeof...(Args) && (sizeof...(Components) > 1))
    decltype(auto) add_component(const entity e, Args&&... args) {
        return std::forward_as_tuple(add_component<Components>(e, std::forward<Args>(args))...);
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
    [[nodiscard]] decltype(auto) get_component(this auto& self, const entity e) noexcept {
        constexpr static std::size_t component_index = meta::index_of<std::remove_cvref_t<Component>, ComponentRegistry...>::value;
        auto& storage = std::get<component_index>(self.storage_map);
        FORGE_ASSERT(storage.contains(e), "[SPARSE GET ASSERTION FAILED] Entity " << e << " is not contained in the sparse storage of component index " << component_index);
        if constexpr (std::is_const_v<Component>)
            return std::as_const(storage.get(e));
        else
            return storage.get(e);
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
    [[nodiscard]] decltype(auto) get_component(this auto& self, const entity e) noexcept {
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
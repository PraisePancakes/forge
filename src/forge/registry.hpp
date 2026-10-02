#pragma once
#include <iostream>
#include <stack>
#include <tuple>
#include <type_traits>
#include <utility>

#include "config/entity_configuration.hpp"
#include "entity.hpp"
#include "generation.hpp"
#include "meta.hpp"
#include "storage.hpp"
#include "view.hpp"
namespace forge {

template <typename... Args>
class signal {
    std::vector<std::function<void(Args...)>> callbacks;

   public:
    template <typename F>
    void connect(F&& f) {
        callbacks.emplace_back(std::forward<F>(f));
    };

    void publish(Args... args) {
        for (auto& callback : callbacks) {
            callback(args...);
        }
    }
};

using entity = configuration::standard_entity;

template <typename... ComponentRegistry>
    requires(meta::is_unique_set_v<ComponentRegistry...> && !meta::contains_it<bool, ComponentRegistry...>)
class registry {
    template <typename T>
    struct component_signals {
        signal<forge::entity, T&> construct;
        signal<forge::entity, T&> update;
        signal<forge::entity, T&> destroy;
    };

    template <typename T>
    using sparse_set_t = storage::sparse_set<T, entity>;

    template <typename T>
    static constexpr std::size_t index_of_type = meta::index_of<std::remove_cvref_t<T>, std::tuple<ComponentRegistry...>>::value;

    template <std::size_t Index>
    using type_of_t = meta::type_of_t<Index, ComponentRegistry...>;

    template <typename... Ts>
    using storage_pool_type = std::tuple<sparse_set_t<type_of_t<index_of_type<Ts>>>...>;
    storage_pool_type<ComponentRegistry...> storage_map;

    entity::id_type current_entity_id{0};
    entity::value_type generate_next() noexcept {
        return static_cast<entity::value_type>(current_entity_id++) << std::numeric_limits<entity::version_type>::digits;
    };
    forge::generation::generator<entity> gen;
    std::tuple<component_signals<ComponentRegistry>...> signal_map;

   public:
    registry() = default;
    registry(const registry&) = delete;
    registry& operator=(const registry&) = delete;
    registry(registry&&) = default;
    registry& operator=(registry&&) = default;

    template <typename Component>
        requires(meta::contains_it<std::remove_cvref_t<Component>, ComponentRegistry...>)
    signal<entity, Component&>& on_construct() {
        return std::get<index_of_type<Component>>(signal_map).construct;
    };

    template <typename Component>
        requires(meta::contains_it<std::remove_cvref_t<Component>, ComponentRegistry...>)
    signal<entity, Component&>& on_destroy() {
        return std::get<index_of_type<Component>>(signal_map).destroy;
    };

    template <typename Component>
        requires(meta::contains_it<std::remove_cvref_t<Component>, ComponentRegistry...>)
    signal<entity, Component&>& on_update() {
        return std::get<index_of_type<Component>>(signal_map).update;
    };

    template <typename... Ts>
        requires(sizeof...(Ts) <= sizeof...(ComponentRegistry))
    [[nodiscard]] decltype(auto) view() noexcept {
        static_assert((meta::contains_it<std::remove_cvref_t<Ts>, ComponentRegistry...> && ...), "Error: Provided view type(s) is not a subset of the world's component registry!");
        return view_span<entity, storage_pool_type<ComponentRegistry...>, std::tuple<Ts...>, std::tuple<>>(this->storage_map);
    }
    /**
     * @brief creates a new entity identifier unless an identifier can be recycled then we use the next version of the recycled identifier.
     */
    entity make() {
        return gen.make();
    };

    [[nodiscard]] bool is_alive(const entity e) const noexcept {
        return gen.is_valid(e);
    }

    template <typename C>
    [[nodiscard]] bool has_component(const entity e) const noexcept {
        if (!is_alive(e))
            return false;
        using stripped_type = std::remove_cvref_t<C>;
        const sparse_set_t<stripped_type>& storage = std::get<meta::index_of<stripped_type, std::tuple<ComponentRegistry...>>::value>(storage_map);
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

    template <typename T>
    bool remove_component(const entity e) noexcept {
        using Component = std::remove_cvref_t<T>;
        static_assert(meta::contains_it<Component, ComponentRegistry...> && "Unregistered component.");
        auto& storage = std::get<meta::index_of<Component, std::tuple<ComponentRegistry...>>::value>(storage_map);
        if (!storage.contains(e)) return false;
        Component& component = storage.get(e);
        on_destroy<Component>().publish(e, component);
        storage.remove(e);
        return true;
    };

    template <typename... Ts>
        requires(sizeof...(Ts) > 1)
    bool remove_component(const entity e) noexcept {
        return (remove_component<Ts>(e) && ...);
    };

    // destroy the entity and remove all its components from the sparse set storage
    void destroy(const entity e) {
        if (!is_alive(e)) return;
        std::apply([&e](auto&&... sets) { (sets.remove(e), ...); }, this->storage_map);
        gen.destroy(e);
    };

    /**
     * @brief adds component of given type, component must be registered to the ComponentRegistry
     *
     * @tparam Component type of component to add.
     * @tparam Args variadic types of arguments
     * @param e the entity to add the component to
     * @param Args variadic list of arguments to forward to in-place component construction.
     */
    template <typename C, typename... Args>
        requires((std::is_object_v<C> || std::is_destructible_v<C>))
    C& add_component(const entity e, Args&&... args) {
        using Component = std::remove_cvref_t<C>;
        static_assert(meta::contains_it<Component, ComponentRegistry...> && "Unregistered component.");
        sparse_set_t<Component>& storage = std::get<meta::index_of<Component, std::tuple<ComponentRegistry...>>::value>(storage_map);
        storage.emplace(e, std::forward<Args>(args)...);
        Component& component = storage.get(e);
        on_construct<Component>().publish(e, component);
        return storage.get(e);
    };

    template <typename... Components, typename... Args>
        requires(sizeof...(Components) == sizeof...(Args) && (sizeof...(Components) > 1))
    decltype(auto) add_component(const entity e, Args&&... args) {
        return std::forward_as_tuple(add_component<Components>(e, std::forward<Args>(args))...);
    };

    template <typename Component, typename... Args>
    Component& replace_component(const entity e, Args&&... args) {
        static_assert(meta::contains_it<Component, ComponentRegistry...> && "Unregistered component.");
        constexpr static std::size_t component_index = meta::index_of<std::remove_cvref_t<Component>, std::tuple<ComponentRegistry...>>::value;
        sparse_set_t<Component>& storage = std::get<meta::index_of<Component, std::tuple<ComponentRegistry...>>::value>(storage_map);
        FORGE_ASSERT(storage.contains(e), "[SPARSE GET ASSERTION FAILED] Entity " << e << " is not contained in the sparse storage of component index " << component_index);
        storage.replace(e, std::forward<Args>(args)...);
        Component& c = storage.get(e);
        on_update<Component>().publish(e, c);
        return storage.get(e);
    };

    template <typename... Components, typename... Args>
        requires(sizeof...(Components) == sizeof...(Args) && (sizeof...(Components) > 1))
    decltype(auto) replace_component(const entity e, Args&&... args) {
        return std::forward_as_tuple(replace_component<Components>(e, std::forward<Args>(args))...);
    };

    template <typename Component, typename... Args>
    Component& add_or_replace_component(const entity e, Args&&... args) {
        if (!has_component<Component>(e)) return add_component<Component>(e, std::forward<Args>(args)...);
        return replace_component<Component>(e, std::forward<Args>(args)...);
    };

    template <typename... Components, typename... Args>
        requires(sizeof...(Components) == sizeof...(Args) && (sizeof...(Components) > 1))
    decltype(auto) add_or_replace_component(const entity e, Args&&... args) {
        return std::forward_as_tuple(add_or_replace_component<Components>(e, std::forward<Args>(args))...);
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
        static_assert(meta::contains_it<std::remove_cvref_t<Component>, ComponentRegistry...> && "Unregistered component.");
        constexpr static std::size_t component_index = meta::index_of<std::remove_cvref_t<Component>, std::tuple<ComponentRegistry...>>::value;
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
    template <typename Component>
    [[nodiscard]] Component* try_get(const entity e) noexcept {
        if (has_component<Component>(e)) return &get_component<Component>(e);
        return nullptr;
    };
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
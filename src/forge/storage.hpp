#pragma once
#include "entity/sparse_set.hpp"

namespace forge::storage {
template <typename Entity, typename Component>
class pool_storage : public basic_sparse_set<Entity> {
    using underlying_container = basic_sparse_set<Entity>;
    using Traits = entity_traits<Entity>;
    std::vector<Component> pool;

   public:
    using value_type = Component;
    using iterator = underlying_container::iterator;
    pool_storage() : pool{} {};
    template <typename... Args>
    void emplace(const Entity e, Args&&... args) {
        FORGE_ASSERT(!this->contains(e), "Cannot emplace on existing component");
        this->push(e);
        pool.emplace_back(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void replace(const Entity e, Args&&... args) {
        FORGE_ASSERT(this->contains(e), "Cannot replace component that does not exist");
        const auto index = this->index_of(e);

        std::destroy_at(std::addressof(pool[index]));
        std::construct_at(std::addressof(pool[index]), std::forward<Args>(args)...);
    }
    void reserve(std::size_t n) {
        this->reserve(n);
        pool.reserve(n);
    }

    Component& get(const Entity e) {
        const auto index = this->index_of(e);
        return pool[index];
    }

    const Component& get(const Entity e) const {
        const auto index = this->index_of(e);
        return pool[index];
    }

    Component& operator[](const Entity e) noexcept {
        return this->get(e);
    }

    void remove(const Entity e) {
        if (!this->contains(e)) return;
        const auto index = this->index_of(e);
        const auto last = pool.size() - 1;
        if (index != last) {
            pool[index] = std::move(pool[last]);
        }
        pool.pop_back();
        underlying_container::remove(e);
    }

    ~pool_storage() {};
};
};  // namespace forge::storage
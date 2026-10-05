#pragma once
#include "entity/sparse_set.hpp"

namespace forge::storage {

template <typename Entity, typename Component, typename Alloc = std::allocator<Component>>

class pool_storage : public basic_sparse_set<Entity> {
    using alloc_traits = std::allocator_traits<Alloc>;
    using underlying_container = basic_sparse_set<Entity>;
    using Traits = entity_traits<Entity>;

    std::vector<Component*> pages;
    Alloc allocator;
    void release_pages() noexcept {
        const auto size = this->size();
        for (std::size_t i = 0; i < size; ++i) {
            std::destroy_at(std::addressof(pages[i / Traits::page_size][i % Traits::page_size]));
        }
        for (auto* page : pages) {
            if (page)
                alloc_traits::deallocate(allocator, page, Traits::page_size);
        }
    }
    Component& component_at(std::size_t index) noexcept {
        const auto page_index = index / Traits::page_size;
        const auto offset = index % Traits::page_size;
        return pages[page_index][offset];
    }

    const Component& component_at(std::size_t index) const noexcept {
        const auto page_index = index / Traits::page_size;
        const auto offset = index % Traits::page_size;

        return pages[page_index][offset];
    }

   public:
    using value_type = Component;

    pool_storage() : pages{}, allocator{} {};
    pool_storage(Alloc allocator) : pages{}, allocator{allocator} {}
    template <typename... Args>
    void emplace(const Entity e, Args&&... args) {
        FORGE_ASSERT(!this->contains(e), "Cannot emplace on existing component");
        this->push(e);
        const auto index = this->index_of(e);
        const auto page_index = index / Traits::page_size;
        if (page_index >= pages.size()) {
            auto* page = alloc_traits::allocate(allocator, Traits::page_size);
            pages.push_back(page);
        }
        std::construct_at(std::addressof(component_at(index)), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void replace(const Entity e, Args&&... args) {
        FORGE_ASSERT(this->contains(e), "Cannot replace component that does not exist");
        const auto index = this->index_of(e);
        std::destroy_at(std::addressof(component_at(index)));
        std::construct_at(std::addressof(component_at(index)), std::forward<Args>(args)...);
    }

    void reserve(std::size_t n) {
        underlying_container::reserve(n);
    }

    Component& get(const Entity e) {
        FORGE_ASSERT(this->contains(e), "getting invalid entity index: entity is not contained in storage");
        return component_at(this->index_of(e));
    }

    const Component& get(const Entity e) const {
        FORGE_ASSERT(this->contains(e), "getting invalid entity index: entity is not contained in storage");
        return component_at(this->index_of(e));
    }

    Component& operator[](const Entity e) noexcept {
        return component_at(this->index_of(e));
    }

    void remove(const Entity e) {
        if (!this->contains(e)) return;

        const auto index = this->index_of(e);
        const auto last = this->size() - 1;

        if (index != last) {
            auto& dst = component_at(index);
            auto& src = component_at(last);
            std::destroy_at(std::addressof(dst));
            std::construct_at(std::addressof(dst), std::move(src));
            std::destroy_at(std::addressof(src));
        } else
            std::destroy_at(std::addressof(component_at(last)));
        underlying_container::remove(e);
    }
    ~pool_storage() {
        release_pages();
    };
};

};  // namespace forge::storage
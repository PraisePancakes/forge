#pragma once
#include <iostream>
#include <vector>

#include "entity.hpp"
#define PAGE_SIZE 256
namespace forge {

template <typename Entity, typename Allocator = std::allocator<Entity>>
class basic_sparse_set {
    using alloc_traits = std::allocator_traits<Allocator>;
    using sparse_type = std::vector<typename alloc_traits::pointer, alloc_traits::template rebind_alloc<typename alloc_traits::pointer>>;
    using dense_type = std::vector<Entity>;
    using Traits = entity_traits<Entity>;
    using size_type = std::size_t;
    sparse_type sparse;
    dense_type dense;

    [[nodiscard]] auto pos_of(const Entity e) const noexcept {
        return static_cast<size_type>(Traits::to_entity(e));
    };

    [[nodiscard]] auto page_of(const size_type pos) const noexcept {
        return static_cast<size_type>(pos / PAGE_SIZE);
    }
    [[nodiscard]] auto offset_of(const size_type pos) const noexcept {
        return static_cast<size_type>(pos % PAGE_SIZE);
    }
    [[nodiscard]] auto get(const Entity e) noexcept {
        auto pos = pos_of(e);
        auto page = page_of(pos);
        return (page < sparse.size() && sparse[page]) ? sparse[page] + offset_of(pos) : nullptr;
    };
    [[nodiscard]] auto& get_ref(const Entity e) noexcept {
        auto* p = get(e);
        FORGE_ASSERT(p, "Invalid reference to null page offset");
        return *p;
    }
    
};
};  // namespace forge
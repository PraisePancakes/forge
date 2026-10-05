#pragma once
#include <iostream>
#include <vector>

#include "entity.hpp"
#define PAGE_SIZE 256
namespace forge {

template <typename Cont>
struct sparse_set_iterator {
    using value_type = Cont::value_type;
    using pointer = Cont::const_pointer;
    using reference = Cont::const_reference;
    using difference_type = Cont::difference_type;
    using iterator_category = std::random_access_iterator_tag;
    const Cont* dense;
    difference_type it{0};

    constexpr sparse_set_iterator() noexcept
        : dense{}, it{} {};
    constexpr sparse_set_iterator(const Cont& c, const difference_type i)
        : dense{&c}, it{i} {};
    constexpr sparse_set_iterator(const sparse_set_iterator& o)
        : dense{o.dense}, it{o.it} {};

    constexpr sparse_set_iterator& operator++() noexcept {
        return (++it, *this);
    }
    constexpr sparse_set_iterator operator++(int) noexcept {
        const sparse_set_iterator og = *this;
        ++(*this);
        return og;
    }

    constexpr sparse_set_iterator& operator--() noexcept {
        return (--it, *this);
    }
    constexpr sparse_set_iterator operator--(int) noexcept {
        const sparse_set_iterator og = *this;
        return (--(*this), og);
    }
    constexpr sparse_set_iterator& operator+=(const difference_type v) noexcept {
        return (it += v, *this);
    }
    constexpr sparse_set_iterator operator+(const difference_type v) const noexcept {
        sparse_set_iterator c = *this;
        return (c += v);
    }
    constexpr sparse_set_iterator& operator-=(const difference_type v) noexcept {
        return (*this += -v);
    }
    constexpr sparse_set_iterator operator-(const difference_type v) const noexcept {
        sparse_set_iterator c = *this;
        return (c -= v);
    }
    [[nodiscard]] constexpr pointer operator->() const noexcept {
        return std::addressof(operator[](0));
    }
    [[nodiscard]] constexpr reference operator[](const difference_type v) const noexcept {
        return (*dense)[static_cast<Cont::size_type>(it + v)];
    }
    [[nodiscard]] constexpr reference operator*() const noexcept {
        return operator[](0);
    }
    [[nodiscard]] constexpr bool operator==(const sparse_set_iterator& o) const noexcept {
        return this->it == o.it;
    }
    [[nodiscard]] constexpr bool operator!=(const sparse_set_iterator& o) const noexcept {
        return !(*this == o);
    }
};

template <typename Entity, typename Allocator = std::allocator<Entity>>
class basic_sparse_set {
    using alloc_traits = std::allocator_traits<Allocator>;
    using sparse_type = std::vector<typename alloc_traits::pointer, typename alloc_traits::template rebind_alloc<typename alloc_traits::pointer>>;
    using dense_type = std::vector<Entity>;
    using Traits = entity_traits<Entity>;
    sparse_type sparse;
    dense_type dense;

    [[nodiscard]] auto pos_of(const Entity e) const noexcept {
        return static_cast<size_type>(Traits::to_entity(e));
    };

    [[nodiscard]] auto page_of(const std::size_t pos) const noexcept {
        return static_cast<std::size_t>(pos / PAGE_SIZE);
    }
    [[nodiscard]] auto offset_of(const std::size_t pos) const noexcept {
        return static_cast<std::size_t>(pos % PAGE_SIZE);
    }
    [[nodiscard]] auto get(const Entity e) noexcept {
        auto pos = pos_of(e);
        auto page = page_of(pos);
        return (page < sparse.size() && sparse[page]) ? sparse[page] + offset_of(pos) : nullptr;
    };
    [[nodiscard]] auto get(const Entity e) const noexcept {
        auto pos = pos_of(e);
        auto page = page_of(pos);
        return (page < sparse.size() && sparse[page]) ? sparse[page] + offset_of(pos) : nullptr;
    };

    // gets position of entity in sparse set, which holds the lookup position in dense
    [[nodiscard]] auto& get_ref(const Entity e) noexcept {
        auto* p = get(e);
        FORGE_ASSERT(p, "Invalid reference to null page offset");
        return *p;
    }

    [[nodiscard]] auto& get_ref(const Entity e) const noexcept {
        auto* p = get(e);
        FORGE_ASSERT(p, "Invalid reference to null page offset");
        return *p;
    }

    void swap_at(const std::size_t l, const std::size_t r) noexcept {
        const auto from = dense[l];
        const auto to = dense[r];
        std::swap(dense[l], dense[r]);
        std::swap(get_ref(from), get_ref(to));
    }

    void swap_and_pop(const Entity e) noexcept {
        if (!contains(e)) {
            return;
        }
        swap_at(get_ref(e), dense.size() - 1);
        get_ref(e) = null;
        dense.pop_back();
    };

    void release_pages() noexcept {
        for (auto alloc{dense.get_allocator()}; auto&& p : sparse) {
            std::destroy(p, p + PAGE_SIZE);
            alloc_traits::deallocate(alloc, p, PAGE_SIZE);
            p = nullptr;
        };
    };
    [[nodiscard]] auto& assure_minimum(const Entity e) {
        auto pos = pos_of(e);
        auto page = page_of(pos);
        if (page >= sparse.size()) {
            sparse.resize(page + 1, nullptr);
        }
        if (!sparse[page]) {
            constexpr typename Traits::value_type def = forge::null;
            auto page_alloc = dense.get_allocator();
            sparse[page] = alloc_traits::allocate(page_alloc, PAGE_SIZE);
            std::uninitialized_fill(sparse[page], sparse[page] + PAGE_SIZE, def);
        }

        return sparse[page][offset_of(pos)];
    };

   public:
    using allocator_type = Allocator;
    using entity_type = Traits::value_type;
    using version_type = Traits::version_type;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = dense_type::const_pointer;
    using iterator = sparse_set_iterator<dense_type>;

   protected:
    iterator emplace(const Entity e) noexcept {
        auto pos = dense.size();
        auto& elem = assure_minimum(e);
        elem = pos;
        dense.push_back(e);
        return sparse_set_iterator{dense, static_cast<difference_type>(pos)};
    };

    [[nodiscard]] auto& index_of(const Entity e) noexcept {
        return get_ref(e);
    };

    [[nodiscard]] auto& index_of(const Entity e) const noexcept {
        return get_ref(e);
    };

   public:
    iterator push(const Entity e) noexcept {
        return emplace(e);
    }

    void reserve(const std::size_t n) noexcept {
        this->dense.reserve(n);
        this->sparse.reserve(n % PAGE_SIZE);
    }
    // must contain an entity of the same identifier and version
    [[nodiscard]] bool contains(const Entity e) const noexcept {
        const auto* p = get(e);
        return (p && *p != null && (forge::to_entity(dense[*p]) == forge::to_entity(e)) && (forge::to_version(dense[*p]) == forge::to_version(e)));
    };

    // lookup of entity in dense using get_ref(e) which gets its dense index from sparse
    [[nodiscard]] auto& operator[](const Entity e) noexcept {
        return dense[get_ref(e)];
    };

    void remove(const Entity e) noexcept {
        swap_and_pop(e);
    };
    // ideally any extended type from this container should define what they want to iterate,
    // they can either iterate the entities or the extended dense items
    // iterator to beginning of dense entities
    iterator begin() {
        return iterator{this->dense, 0};
    };
    // iterator to end of dense entities
    iterator end() {
        return iterator{this->dense, static_cast<difference_type>(this->dense.size())};
    };

    [[nodiscard]] std::size_t size() const noexcept {
        return this->dense.size();
    }

    ~basic_sparse_set() {
        release_pages();
    }
};
};  // namespace forge
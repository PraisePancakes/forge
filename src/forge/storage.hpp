#pragma once

#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>
#define PAGE_SIZE 256
namespace forge::storage {

template <typename CTy, typename KeyType>
    requires(!std::is_same_v<CTy, bool>)
class sparse_set {
    struct page {
        alignas(CTy) std::byte storage[sizeof(CTy) * PAGE_SIZE];

        [[nodiscard]] CTy* data() noexcept {
            return reinterpret_cast<CTy*>(storage);
        }

        [[nodiscard]] const CTy* data() const noexcept {
            return reinterpret_cast<const CTy*>(storage);
        }
        // safe read on reinterpret cast
        [[nodiscard]] CTy& operator[](std::size_t index) noexcept {
            return *std::launder(data() + index);
        }

        [[nodiscard]] const CTy& operator[](std::size_t index) const noexcept {
            return *std::launder(data() + index);
        }
    };

    std::vector<std::unique_ptr<page>> components;
    std::vector<KeyType> dense_mirror;
    std::vector<std::size_t> sparse;

    void assure_page(std::size_t index) {
        const auto page_index = index / PAGE_SIZE;

        if (page_index >= components.size()) {
            components.resize(page_index + 1);
        }

        if (!components[page_index]) {
            components[page_index] = std::make_unique<page>();
        }
    }

    [[nodiscard]] CTy& at(std::size_t index) noexcept {
        return (*components[index / PAGE_SIZE])[index % PAGE_SIZE];
    }

    [[nodiscard]]
    const CTy& at(std::size_t index) const noexcept {
        return (*components[index / PAGE_SIZE])[index % PAGE_SIZE];
    }

   public:
    using value_type = CTy;
    using key_type = KeyType;
    static constexpr std::size_t EMPTY = std::numeric_limits<std::size_t>::max();
    sparse_set() = default;
    ~sparse_set() {
        for (std::size_t i = 0; i < dense_mirror.size(); ++i) {
            std::destroy_at(std::addressof(at(i)));
        }
    }

    sparse_set(const sparse_set&) = delete;
    sparse_set& operator=(const sparse_set&) = delete;

    sparse_set(sparse_set&& other) noexcept = default;
    sparse_set& operator=(sparse_set&& other) noexcept = default;

    template <typename... Args>
    void emplace(KeyType e, Args&&... args) {
        const auto id = to_id(e);
        if (id >= sparse.size()) sparse.resize((id + 1) * 2, EMPTY);
        if (sparse[id] != EMPTY) return;
        const auto index = dense_mirror.size();
        assure_page(index);
        std::construct_at(std::addressof(at(index)), std::forward<Args>(args)...);
        dense_mirror.push_back(e);
        sparse[id] = index;
    }

    template <typename... Args>
    void replace(KeyType e, Args&&... args) {
        FORGE_ASSERT(contains(e), "Cannot replace component that does not exist");
        const auto index = sparse[to_id(e)];
        std::destroy_at(std::addressof(at(index)));
        std::construct_at(std::addressof(at(index)), std::forward<Args>(args)...);
    }

    void reserve(std::size_t n) {
        dense_mirror.reserve(n);
        sparse.reserve(n);
        const auto page_count = (n + PAGE_SIZE - 1) / PAGE_SIZE;
        components.reserve(page_count);
    }

    void remove(KeyType e) {
        if (!contains(e)) return;
        const auto id = to_id(e);
        const auto index = sparse[id];
        const auto last = dense_mirror.size() - 1;
        if (index != last) {
            const auto moved_entity = dense_mirror[last];
            std::destroy_at(std::addressof(at(index)));
            std::construct_at(std::addressof(at(index)), std::move(at(last)));
            std::destroy_at(std::addressof(at(last)));
            dense_mirror[index] = moved_entity;
            sparse[to_id(moved_entity)] = index;
        } else {
            std::destroy_at(std::addressof(at(index)));
        }
        dense_mirror.pop_back();
        sparse[id] = EMPTY;
    }

    [[nodiscard]] auto begin() noexcept {
        return dense_mirror.begin();
    }

    [[nodiscard]] auto end() noexcept {
        return dense_mirror.end();
    }

    [[nodiscard]] auto begin() const noexcept {
        return dense_mirror.begin();
    }

    [[nodiscard]] auto end() const noexcept {
        return dense_mirror.end();
    }

    [[nodiscard]] auto cbegin() const noexcept {
        return dense_mirror.cbegin();
    }

    [[nodiscard]] auto cend() const noexcept {
        return dense_mirror.cend();
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return dense_mirror.size();
    }

    [[nodiscard]] bool contains(KeyType e) const noexcept {
        return to_id(e) < sparse.size() && sparse[to_id(e)] != EMPTY;
    }

    [[nodiscard]] CTy& get(KeyType e) noexcept {
        FORGE_ASSERT(contains(e), "Invalid get, dereferencing null");
        return at(sparse[to_id(e)]);
    }

    [[nodiscard]] const CTy& get(KeyType e) const noexcept {
        FORGE_ASSERT(contains(e), "Invalid get, dereferencing null");
        return at(sparse[to_id(e)]);
    }
};

}  // namespace forge::storage
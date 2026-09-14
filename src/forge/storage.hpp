#pragma once
#include <vector>
namespace forge::storage {
template <typename CTy, typename KeyType>
    requires(!std::is_same_v<CTy, bool>)
class sparse_set {
    std::vector<CTy> dense;
    std::vector<KeyType> dense_mirror;
    std::vector<std::size_t> sparse;

   public:
    using value_type = CTy;
    using key_type = KeyType;

    static constexpr std::size_t EMPTY = std::numeric_limits<std::size_t>::max();

    template <typename... Args>
    void emplace(KeyType e, Args&&... args) {
        if (e >= sparse.size()) {
            sparse.resize(e + 1, EMPTY);
        }
        if (sparse[e] != EMPTY) {
            return;
        }
        sparse[e] = dense.size();
        dense_mirror.push_back(e);
        dense.emplace_back(std::forward<Args>(args)...);
    };
    // swap and pop
    void remove(KeyType e) {
        if (!contains(e)) return;
        std::swap(dense[sparse[e]], dense.back());
        std::swap(dense_mirror[sparse[e]], dense_mirror.back());
        sparse[dense_mirror[sparse[e]]] = sparse[e];
        dense.pop_back();
        dense_mirror.pop_back();
        sparse[e] = EMPTY;
    };

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
        return this->dense_mirror.size();
    }
    [[nodiscard]] bool contains(KeyType e) const noexcept {
        return e < sparse.size() && sparse[e] != EMPTY;
    }

    [[nodiscard]] CTy& get(KeyType e) noexcept {
        return dense[sparse[e]];
    };

    [[nodiscard]] const CTy& get(KeyType e) const noexcept {
        return dense[sparse[e]];
    }
};

};  // namespace forge::storage
#pragma once
#include <vector>
namespace forge::storage {
template <typename CTy, typename KeyType>
class sparse_set {
    std::vector<CTy> dense;
    std::vector<KeyType> dense_mirror;
    std::vector<std::size_t> sparse;

   public:
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

    bool contains(KeyType e) const noexcept {
        return e < sparse.size() && sparse[e] != EMPTY;
    }

    CTy& get(KeyType e) noexcept {
        return dense[sparse[e]];
    };

    const CTy& get(KeyType e) const noexcept {
        return dense[sparse[e]];
    }
};

};  // namespace forge::storage
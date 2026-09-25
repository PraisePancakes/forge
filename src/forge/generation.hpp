#pragma once
#include <iostream>
#include <limits>
#include <stack>
#include <vector>
namespace forge::generation {
template <typename T>
class generator {
    using id_type = T::id_type;
    using version_type = T::version_type;
    using value_type = T::value_type;
    id_type current_T_id{0};
    std::vector<version_type> version_history;
    std::stack<T> store;

   public:
    value_type generate_next() noexcept {
        return static_cast<value_type>(current_T_id++) << std::numeric_limits<version_type>::digits;
    };
    T make() {
        if (!store.empty()) {
            auto e = store.top();
            store.pop();
            return next(e);
        }
        T e{generate_next()};
        version_history.push_back(to_version(e));
        return e;
    };
    [[nodiscard]] bool is_valid(const T e) const noexcept {
        return to_id(e) < version_history.size() &&
               version_history[to_id(e)] == to_version(e);
    }

    void destroy(const T e) {
        if (!is_valid(e))
            return;
        version_history[to_id(e)] = to_version(next(e));
        this->store.push(e);
    }
};
};  // namespace forge::generation
#pragma once
#include <stack>
#include <vector>

#include "entity.hpp"

namespace forge {
template <typename Entity>
struct generator {
    using Traits = entity_traits<Entity>;
    using entity_type = Traits::entity_type;
    using version_type = Traits::version_type;
    using value_type = Traits::value_type;

    std::vector<version_type> version_history;
    std::stack<value_type> store;
    entity_type entity{0};

   public:
    generator() = default;
    generator(const generator&) = default;
    generator(generator&&) = default;
    generator& operator=(generator&&) = default;
    generator& operator=(const generator&) = default;
    ~generator() = default;

    [[nodiscard]] value_type make() noexcept {
        if (!store.empty()) {
            auto e = store.top();
            store.pop();
            return Traits::next(e);
        }
        auto e = Traits::construct(entity++, 0);
        version_history.push_back(forge::to_version(e));
        return e;
    };

    [[nodiscard]] bool valid(const value_type v) const noexcept {
        return forge::to_entity(v) < version_history.size() &&
               version_history[forge::to_entity(v)] == forge::to_version(v);
    };

    void destroy(const value_type v) noexcept {
        if (!valid(v))
            return;
        version_history[forge::to_entity(v)] = forge::to_version(Traits::next(v));
        this->store.push(v);
    }
};
}  // namespace forge
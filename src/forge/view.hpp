#pragma once
#include <functional>
#include <iostream>
#include <tuple>

#include "storage.hpp"
namespace forge {

template <typename KeyType, typename... Queries>
class view_span {
   public:
    using storage_tuple = std::tuple<storage::sparse_set<std::remove_cvref_t<Queries>, KeyType>&...>;
    using key_type = KeyType;

   private:
    storage_tuple storage_span;

    static std::size_t minimum_storage_index(const storage_tuple& span) {
        return std::apply(([](auto&&... args) {
                              std::size_t min = std::numeric_limits<std::size_t>::max();
                              std::size_t idx = 0;
                              std::size_t curr_idx = 0;
                              (([&min, &idx, &curr_idx](auto&& arg) {
                                   auto a = std::forward<decltype(arg)>(arg);
                                   if (a.size() < min) {
                                       min = a.size();
                                       idx = curr_idx;
                                   }
                                   curr_idx++;
                               }(std::forward<decltype(args)>(args))),
                               ...);
                              return idx;
                          }),
                          span);
    }

    bool contains_in_all(key_type key) const {
        return std::apply(([key](auto&&... args) {
                              return ((std::forward<decltype(args)>(args).contains(key)) && ...);
                          }),
                          storage_span);
    };
    template <typename Query, typename Storage>
    static decltype(auto) get(Storage& storage, key_type key) {
        if constexpr (std::is_const_v<std::remove_reference_t<Query>>) {
            return std::as_const(storage).get(key);
        } else {
            return storage.get(key);
        }
    }

    template <typename Func, std::size_t... Is>
    void invoke(Func& callback, key_type e, std::index_sequence<Is...>) {
        callback(get<Queries>(std::get<Is>(storage_span), e)...);
    }
    std::size_t driving_index{0};

   public:
    view_span(storage_tuple sp)
        : storage_span{sp},
          driving_index{minimum_storage_index(sp)} {};

    template <typename Func>
    void each(Func&& callback) {
        meta::tuple_switch(this->driving_index, storage_span, [this, &callback](auto&& storage) {
            for (const auto& e : std::forward<decltype(storage)>(storage)) {
                if (this->contains_in_all(e)) invoke(callback, e, std::index_sequence_for<Queries...>{});
            };
        });
    };
};
};  // namespace forge
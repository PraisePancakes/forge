#pragma once
#include <functional>
#include <iostream>
#include <tuple>

#include "storage.hpp"
namespace forge {

// entity::id_type     ,  const int, char, etc...
template <typename Ty, typename... Queries>
class view_span {
   public:
    using key_type = Ty;
    using pool_reference_type = std::tuple<forge::storage::sparse_set<std::remove_cvref_t<Queries>, Ty>&...>;

   private:
    pool_reference_type packed_storage_pool;
    std::size_t driving_index{0};

    static std::size_t get_driving_index(const pool_reference_type pool) {
        return std::apply([](auto&&... args) {
            std::size_t min_sz = std::numeric_limits<std::size_t>::max();
            std::size_t min_idx = 0;
            std::size_t current_index = 0;
            (([&min_sz, &min_idx, &current_index](auto&& arg) {
                 if (arg.size() < min_sz && arg.size() != 0) {
                     min_sz = arg.size();
                     min_idx = current_index;
                 }
                 current_index++;
             }(args)),
             ...);
            return min_idx;
        },
                          pool);
    };

    static bool contains_in_every(const key_type key, const pool_reference_type pool) {
        return std::apply([&key](auto&&... args) {
            return (args.contains(key) && ...);
        },
                          pool);
    };

    // we must check whether the callback is invocable with an extended key_type if it is call it with the extended key type, otherwise call it witout.
    template <typename Func, std::size_t... Is>
    void propagate_cv_callback(const key_type e, Func& callback, const std::index_sequence<Is...>) {
        callback([&]<std::size_t I>(const std::integral_constant<std::size_t, I>) -> decltype(auto) { 
                        auto& storage = std::get<I>(packed_storage_pool); 
                        using query_type = std::tuple_element_t<I, std::tuple<Queries...>>; 
                        if constexpr (std::is_const_v<std::remove_reference_t<query_type>>) 
                            return std::as_const(storage.get(e)); 
                        else return (storage.get(e)); }(std::integral_constant<std::size_t, Is>{})...);
    }

   public:
    view_span(pool_reference_type pool)
        : packed_storage_pool(pool),
          driving_index{get_driving_index(pool)} {};

    template <typename Func>
    void each(Func&& callback) {
        meta::_INTERNAL::homogeneous_template_tuple_get(driving_index, packed_storage_pool, [&callback, this](const auto& arg) {
            for (const auto& e : arg) {
                if (!contains_in_every(e, this->packed_storage_pool)) continue;
                this->propagate_cv_callback(e, callback, std::make_index_sequence<sizeof...(Queries)>{});
            };
        });
    };
};
};  // namespace forge
#pragma once
#include <iostream>
#include <utility>

#include "meta.hpp"
namespace forge::algorithms {

namespace containers {
// get the driving index (smallest pool of components) to iterate over
template <typename PoolType>
std::size_t get_driving_index(PoolType&& p) {
    std::size_t idx = 0;

    std::apply([&idx](auto&&... args) {
        std::size_t curr_idx = 0;
        std::size_t min = std::numeric_limits<std::size_t>::max();

        ([&](const auto& arg) {
            if (arg.size() > 0 && arg.size() < min) {
                min = arg.size();
                idx = curr_idx;
            }
            ++curr_idx;
        }(args),
         ...);
    },
               std::forward<PoolType>(p));

    return idx;
}

template <typename PoolType, typename... Ts>
decltype(auto) subset_of(PoolType& pool) {
    using Pool = std::remove_cvref_t<PoolType>;
    return std::forward_as_tuple(
        std::get<meta::index_of_value_type<std::remove_cvref_t<Ts>, Pool>::value>(pool)...);
}

template <typename E, typename PoolType>
bool contains_in_every(const E e, PoolType&& p) {
    return std::apply([&e](auto&&... args) {
        return (args.contains(e) && ...);
    },
                      std::forward<PoolType>(p));
}

template <typename E, typename PoolType>
bool contains_in_none(const E e, PoolType&& p) {
    return std::apply([&e](auto&&... args) {
        return (!args.contains(e) && ...);
    },
                      std::forward<PoolType>(p));
}

template <typename PoolType>
bool contains_empty(PoolType pool) {
    return std::apply([](auto&&... pools) {
        return ((pools.size() == 0) || ...);
    },
                      pool);
}
template <std::size_t I, typename E, typename... Ts, typename PoolType>
decltype(auto) pool_of(const E e, PoolType&& pool) {
    auto& storage = std::get<I>(std::forward<PoolType>(pool));
    using query_type = std::tuple_element_t<I, std::tuple<Ts...>>;
    if constexpr (std::is_const_v<std::remove_reference_t<query_type>>) {
        return std::as_const(storage.get(e));
    } else {
        return (storage.get(e));
    }
}

};  // namespace containers

};  // namespace forge::algorithms
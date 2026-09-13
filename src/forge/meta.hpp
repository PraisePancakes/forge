#pragma once
#include <iostream>

namespace forge::meta {
template <typename T, typename... Ts>
constexpr static bool contains_it = (std::is_same_v<T, Ts> || ...);

template <std::size_t I>
using value = std::integral_constant<std::size_t, I>;

template <typename T, typename... Ts>
    requires(contains_it<T, Ts...>)
struct index_of {
    static constexpr std::size_t index =
        []<std::size_t... Is>(std::index_sequence<Is...>) {
            std::size_t result{};
            ((std::is_same_v<T, Ts> ? result = Is : result), ...);
            return result;
        }(std::make_index_sequence<sizeof...(Ts)>{});
};

template <std::size_t Index, typename... Ts>
    requires(Index >= 0 && Index < sizeof...(Ts))
struct type_of;

template <typename Head, typename... Tail>
struct type_of<0, Head, Tail...> {
    using type = Head;
};

template <std::size_t Index, typename Head, typename... Tail>
struct type_of<Index, Head, Tail...> {
    using type = typename type_of<Index - 1, Tail...>::type;
};

template <typename... Ts>
struct is_unique_set;

template <typename T>
struct is_unique_set<T> : std::true_type {};

template <typename T, typename... Ts>
struct is_unique_set<T, Ts...> : std::conditional_t<(std::is_same_v<T, Ts> || ...), std::false_type, is_unique_set<Ts...>> {};

template <template <typename...> class T, typename... Ts>
struct is_unique_set<T<Ts...>> : is_unique_set<Ts...> {};

template <typename... Ts>
constexpr static bool is_unique_set_v = is_unique_set<Ts...>::value;

}  // namespace forge::meta
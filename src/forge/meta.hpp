#pragma once
#include <iostream>
#include <tuple>
namespace forge::meta {
template <typename T, typename... Ts>
constexpr static bool contains_it = (std::is_same_v<T, Ts> || ...);

template <std::size_t I>
using value = std::integral_constant<std::size_t, I>;

template <typename T, typename... Ts>
    requires(contains_it<T, Ts...>)
struct index_of {
    static constexpr std::size_t value =
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

template <std::size_t Index, typename... Ts>
using type_of_t = type_of<Index, Ts...>::type;

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

namespace detail {

template <typename Tup, typename Func, std::size_t... I>
void runtime_tuple_get(const std::size_t index, Tup&& tup, Func&& f, const std::index_sequence<I...>) {
    /*  let index = 1;
        []() { I == 0 && f(tup)} (), <- doesn't call since && short circuits on I != index.
        []() { I == 1 && f(tup)} (), <- calls since && is met
        since f(tup) is not a boolean argument we pass the short circuit, then discard f(tup) with a comma operator to a boolean argument in this case it can be false/true doesnt really matter
    */

    (([&]() { (void)(I == index && (std::forward<Func>(f)(std::get<I>(std::forward<Tup>(tup))), false)); }()), ...);
};

}  // namespace detail

template <typename Tup, typename Func>
void runtime_tuple_get(const std::size_t index, Tup&& t, Func&& f) {
    detail::runtime_tuple_get(index,
                              std::forward<Tup>(t),
                              std::forward<Func>(f),
                              std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tup>>>{});
}
}  // namespace forge::meta
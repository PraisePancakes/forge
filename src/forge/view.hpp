#pragma once
#include <functional>
#include <iostream>
#include <tuple>

#include "algorithms.hpp"
#include "storage.hpp"
namespace forge {
using namespace algorithms;

namespace _INTERNAL::TAGS {
struct deref_row_wise_tag;
struct deref_column_wise_tag;
}  // namespace _INTERNAL::TAGS

template <typename It, typename DerefTag, typename Includes, typename Excludes>
class view_iterator;

template <typename It, typename DerefTag, typename... Includes, typename... Excludes>
class view_iterator<It, DerefTag, std::tuple<Includes...>, std::tuple<Excludes...>> {
    using iterator_traits = std::iterator_traits<It>;
    using value_type = iterator_traits::value_type;
    using Get = std::tuple<forge::storage::pool_storage<value_type, std::remove_cvref_t<Includes>>&...>;
    using Exclude = std::tuple<forge::storage::pool_storage<value_type, std::remove_cvref_t<Excludes>>&...>;
    Get gets;
    Exclude excludes;
    It it{};
    It end{};

   public:
    view_iterator(const std::size_t row_index, Get& gets, Exclude& ex)
        : gets{gets}, excludes{ex} {
        meta::_INTERNAL::homogeneous_template_tuple_get(containers::get_driving_index(gets), gets, [this, row_index](auto&& arg) {
            this->it = arg.begin() + row_index;
            this->end = arg.end();
        });
    };

    view_iterator& operator++() {
        this->it++;
        if constexpr (sizeof...(Includes) == 1 &&
                      sizeof...(Excludes) == 0)
            return *this;
        while (this->it != this->end && (!containers::contains_in_every(*this->it, gets) ||
                                         !containers::contains_in_none(*this->it, excludes))) {
            this->it++;
        }
        return *this;
    }
    view_iterator operator++(int) {
        auto old = *this;
        ++(*this);
        return old;
    }
    value_type get_value() const {
        return *this->it;
    }

    decltype(auto) operator*() {
        if constexpr (std::is_same_v<DerefTag, _INTERNAL::TAGS::deref_column_wise_tag>) {
            return [this]<std::size_t... Is>(std::index_sequence<Is...>) {
                return std::forward_as_tuple(containers::pool_of<Is, value_type, Includes...>(*this->it, this->gets)...);
            }(std::make_index_sequence<sizeof...(Includes)>{});
        } else if constexpr (std::is_same_v<DerefTag, _INTERNAL::TAGS::deref_row_wise_tag>) {
            return this->get_value();
        }
    }

    friend bool operator==(const view_iterator& a, const view_iterator& b) {
        return a.it == b.it;
    }
    friend bool operator!=(const view_iterator& a, const view_iterator& b) {
        return !(a == b);
    }
};
template <typename Entity, typename UniversalPool, typename Includes, typename Excludes>
class basic_view_container;

template <typename Entity, typename UniversalPool, typename... Includes, typename... Excludes>
class basic_view_container<Entity, UniversalPool, std::tuple<Includes...>, std::tuple<Excludes...>> {
    using value_type = Entity;
    using iterator = view_iterator<sparse_set_iterator<std::vector<Entity>>, _INTERNAL::TAGS::deref_column_wise_tag, std::tuple<Includes...>, std::tuple<Excludes...>>;

    using inclusion_type = std::tuple<forge::storage::pool_storage<Entity, std::remove_cvref_t<Includes>>&...>;
    using exclusion_type = std::tuple<forge::storage::pool_storage<Entity, std::remove_cvref_t<Excludes>>&...>;

   protected:
    inclusion_type inclusions;
    exclusion_type exclusions;
    std::size_t driving_index;

   public:
    basic_view_container(UniversalPool& pool)
        : inclusions{containers::subset_of<UniversalPool, Includes...>(pool)},
          exclusions{containers::subset_of<UniversalPool, Excludes...>(pool)},
          driving_index{containers::get_driving_index(containers::subset_of<UniversalPool, Includes...>(pool))} {}
    iterator begin() {
        if (containers::contains_empty(inclusions)) return end();
        return iterator{0, inclusions, exclusions};
    };

    const iterator begin() const {
        if (containers::contains_empty(inclusions)) return end();
        return iterator{0, inclusions, exclusions};
    };

    iterator end() {
        std::size_t end_index = 0;
        meta::_INTERNAL::homogeneous_template_tuple_get(this->driving_index, inclusions, [&end_index](auto&& arg) {
            end_index = arg.size();
        });
        return iterator{end_index, inclusions, exclusions};
    };

    const iterator end() const {
        std::size_t end_index = 0;
        meta::_INTERNAL::homogeneous_template_tuple_get(this->driving_index, inclusions, [&end_index](auto&& arg) {
            end_index = arg.size();
        });
        return iterator{end_index, inclusions, exclusions};
    };
};

template <typename E, typename UniversalPool, typename Include, typename Exclude>
class view_fwd;

template <typename E, typename UniversalPool, typename... Includes, typename... Excludes>
class view_fwd<E, UniversalPool, std::tuple<Includes...>, std::tuple<Excludes...>>
    : private basic_view_container<E, UniversalPool, std::tuple<Includes...>, std::tuple<Excludes...>> {
    using underlying_container = basic_view_container<E, UniversalPool, std::tuple<Includes...>, std::tuple<Excludes...>>;
    using iterator = view_iterator<sparse_set_iterator<std::vector<E>>, _INTERNAL::TAGS::deref_row_wise_tag, std::tuple<Includes...>, std::tuple<Excludes...>>;

    template <typename Func, std::size_t... Is>
    void propogate_const_callback(const E e, Func& callback, const std::index_sequence<Is...>) {
        if constexpr (std::is_invocable_v<Func, E, decltype(containers::pool_of<Is, E, Includes...>(e, this->inclusions))...>) {
            callback(e, containers::pool_of<Is, E, Includes...>(e, this->inclusions)...);
        } else if constexpr (std::is_invocable_v<Func, decltype(containers::pool_of<Is, E, Includes...>(e, this->inclusions))...>) {
            callback(containers::pool_of<Is, E, Includes...>(e, this->inclusions)...);
        }
    }

   public:
    underlying_container& each() {
        return *this;
    };

    view_fwd(UniversalPool& pool) : underlying_container{pool} {};

    const iterator begin() const {
        if (containers::contains_empty(this->inclusions)) return end();
        return iterator{0, this->inclusions, this->exclusions};
    };

    const iterator end() const {
        std::size_t end_index = 0;
        meta::_INTERNAL::homogeneous_template_tuple_get(this->driving_index, this->inclusions, [&end_index](auto&& arg) {
            end_index = arg.size();
        });
        return iterator{end_index, this->inclusions, this->exclusions};
    };

    iterator begin() {
        if (containers::contains_empty(this->inclusions)) return end();
        return iterator{0, this->inclusions, this->exclusions};
    };

    iterator end() {
        std::size_t end_index = 0;
        meta::_INTERNAL::homogeneous_template_tuple_get(this->driving_index, this->inclusions, [&end_index](auto&& arg) {
            end_index = arg.size();
        });
        return iterator{end_index, this->inclusions, this->exclusions};
    };

    template <typename Func>
    void each(Func&& f) {
        if constexpr (sizeof...(Includes) == 1 && sizeof...(Excludes) == 0) {
            // native iteration
            auto& pool = std::get<0>(this->inclusions);
            for (auto& e : pool) {
                this->propogate_const_callback(e, f, std::make_index_sequence<1>{});
            }
        } else {
            auto it = underlying_container::begin();
            auto last = underlying_container::end();
            for (; it != last; ++it) {
                this->propogate_const_callback(it.get_value(), f, std::make_index_sequence<sizeof...(Includes)>{});
            }
        }
    };
};

template <typename E, typename UniversalPool, typename Include, typename Exclude>
class view_span;

template <typename E, typename UniversalPool, typename... Includes>
class view_span<E, UniversalPool, std::tuple<Includes...>, std::tuple<>>
    : public view_fwd<E, UniversalPool, std::tuple<Includes...>, std::tuple<>> {
    using underlying_container = view_fwd<E, UniversalPool, std::tuple<Includes...>, std::tuple<>>;
    UniversalPool& pool_ref;

   public:
    view_span(UniversalPool& pool) : underlying_container{pool}, pool_ref{pool} {};

    template <typename... Excludes>
    auto exclude() {
        return view_fwd<E, UniversalPool, std::tuple<Includes...>, std::tuple<Excludes...>>(this->pool_ref);
    };
};
};  // namespace forge
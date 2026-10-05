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

};  // namespace forge
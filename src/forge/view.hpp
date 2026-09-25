#pragma once
#include <functional>
#include <iostream>
#include <tuple>

#include "algorithms.hpp"
#include "storage.hpp"
namespace forge {
using namespace forge::algorithms;
namespace _INTERNAL::TAGS {
struct deref_row_wise_tag;
struct deref_column_wise_tag;
}  // namespace _INTERNAL::TAGS

template <typename T>
class basic_view_iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using const_pointer = const T*;
    using const_reference = const T&;

   private:
    pointer ptr;

   public:
    basic_view_iterator(pointer p) : ptr{p} {};

    friend bool operator==(const basic_view_iterator& a, const basic_view_iterator& b) {
        return a.ptr == b.ptr;
    }
    friend bool operator!=(const basic_view_iterator& a, const basic_view_iterator& b) {
        return !(a == b);
    }

    basic_view_iterator& operator++() {
        this->ptr++;
        return *this;
    }
    basic_view_iterator operator++(int) {
        auto old = *this;
        ++(*this);
        return old;
    }
    reference operator*() const {
        return *ptr;
    }

    pointer operator->() const {
        return ptr;
    }
};

template <typename DerefTag, typename BaseIterator, typename Include, typename Exclude>
class view_iterator;

template <typename DerefTag, typename BaseIterator, typename... InclusionPools, typename... ExclusionPools>
class view_iterator<DerefTag, BaseIterator, std::tuple<InclusionPools...>, std::tuple<ExclusionPools...>> {
    using iterator_traits = std::iterator_traits<BaseIterator>;
    using value_type = iterator_traits::value_type;
    using iterator = BaseIterator;
    using entity_type = std::remove_cvref_t<value_type>;

    using inclusion_type = std::tuple<forge::storage::sparse_set<std::remove_cvref_t<InclusionPools>, entity_type>&...>;

    using exclusion_type = std::tuple<forge::storage::sparse_set<std::remove_cvref_t<ExclusionPools>, entity_type>&...>;
    iterator iter{nullptr};
    iterator end_iter{nullptr};
    inclusion_type inclusions;
    exclusion_type exclusions;

   public:
    view_iterator(const std::size_t driving_index, const std::size_t row_index, inclusion_type& ipool, exclusion_type& expool)
        : inclusions{ipool},
          exclusions{expool} {
        meta::_INTERNAL::homogeneous_template_tuple_get(driving_index, ipool, [this, row_index](auto&& arg) {
            this->iter = BaseIterator{std::to_address(arg.begin() + row_index)};
            this->end_iter = BaseIterator{std::to_address(arg.end())};
        });
    };
    view_iterator& operator++() {
        this->iter++;
        while (this->iter != this->end_iter && (!containers::contains_in_every(*this->iter, inclusions) ||
                                                !containers::contains_in_none(*this->iter, exclusions))) {
            this->iter++;
        }
        return *this;
    }

    view_iterator operator++(int) {
        auto old = *this;
        ++(*this);
        return old;
    }

    value_type get_value() const {
        return *this->iter;
    }

    decltype(auto) operator*() {
        if constexpr (std::is_same_v<DerefTag, _INTERNAL::TAGS::deref_column_wise_tag>) {
            return [this]<std::size_t... Is>(std::index_sequence<Is...>) {
                return std::forward_as_tuple(containers::pool_of<Is, value_type, InclusionPools...>(*this->iter, this->inclusions)...);
            }(std::make_index_sequence<sizeof...(InclusionPools)>{});
        } else if constexpr (std::is_same_v<DerefTag, _INTERNAL::TAGS::deref_row_wise_tag>) {
            return this->get_value();
        }
    }

    friend bool operator==(const view_iterator& a, const view_iterator& b) {
        return a.iter == b.iter;
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
    using iterator = view_iterator<_INTERNAL::TAGS::deref_column_wise_tag, basic_view_iterator<Entity>, std::tuple<Includes...>, std::tuple<Excludes...>>;

    using inclusion_type = std::tuple<forge::storage::sparse_set<std::remove_cvref_t<Includes>, Entity>&...>;
    using exclusion_type = std::tuple<forge::storage::sparse_set<std::remove_cvref_t<Excludes>, Entity>&...>;

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
        return iterator{this->driving_index, 0,
                        inclusions,
                        exclusions};
    };

    const iterator begin() const {
        if (containers::contains_empty(inclusions)) return end();
        return iterator{this->driving_index, 0,
                        inclusions,
                        exclusions};
    };

    iterator end() {
        std::size_t end_index = 0;
        meta::_INTERNAL::homogeneous_template_tuple_get(this->driving_index, inclusions, [&end_index](auto&& arg) {
            end_index = arg.size();
        });
        return iterator{this->driving_index, end_index, inclusions, exclusions};
    };

    const iterator end() const {
        std::size_t end_index = 0;
        meta::_INTERNAL::homogeneous_template_tuple_get(this->driving_index, inclusions, [&end_index](auto&& arg) {
            end_index = arg.size();
        });
        return iterator{this->driving_index, end_index,
                        inclusions,
                        exclusions};
    };
};

template <typename E, typename UniversalPool, typename Include, typename Exclude>
class view_span;

template <typename E, typename UniversalPool, typename... Includes>
class view_span<E, UniversalPool, std::tuple<Includes...>, std::tuple<>>
    : private basic_view_container<E, UniversalPool, std::tuple<Includes...>, std::tuple<>> {
    using underlying_container = basic_view_container<E, UniversalPool, std::tuple<Includes...>, std::tuple<>>;
    using iterator = view_iterator<_INTERNAL::TAGS::deref_row_wise_tag, basic_view_iterator<E>, std::tuple<Includes...>, std::tuple<>>;

    template <typename Func, std::size_t... Is>
    void propagate_cv_callback(const E e, Func& callback, const std::index_sequence<Is...>) {
        if constexpr (std::is_invocable_v<Func, E, decltype(containers::pool_of<Is, E, Includes...>(e, this->inclusions))...>) {
            callback(e, containers::pool_of<Is, E, Includes...>(e, this->inclusions)...);
        } else if constexpr (std::is_invocable_v<Func, decltype(containers::pool_of<Is, E, Includes...>(e, this->inclusions))...>) {
            callback(containers::pool_of<Is, E, Includes...>(e, this->inclusions)...);
        }
    }

   public:
    view_span(UniversalPool& pool) : underlying_container{pool} {};
    underlying_container& each() {
        return *this;
    };

    const iterator begin() const {
        if (containers::contains_empty(this->inclusions)) return end();
        return iterator{this->driving_index, 0, this->inclusions, this->exclusions};
    };

    const iterator end() const {
        std::size_t end_index = 0;
        meta::_INTERNAL::homogeneous_template_tuple_get(this->driving_index, this->inclusions, [&end_index](auto&& arg) {
            end_index = arg.size();
        });
        return iterator{this->driving_index, end_index, this->inclusions, this->exclusions};
    };

    iterator begin() {
        if (containers::contains_empty(this->inclusions)) return end();
        return iterator{this->driving_index, 0, this->inclusions, this->exclusions};
    };

    iterator end() {
        std::size_t end_index = 0;
        meta::_INTERNAL::homogeneous_template_tuple_get(this->driving_index, this->inclusions, [&end_index](auto&& arg) {
            end_index = arg.size();
        });
        return iterator{this->driving_index, end_index, this->inclusions, this->exclusions};
    };

    template <typename Func>
    void each(Func&& f) {
        for (auto it = underlying_container::begin(); it != underlying_container::end(); it++) {
            this->propagate_cv_callback(it.get_value(), f, std::make_index_sequence<sizeof...(Includes)>{});
        }
    };
};

};  // namespace forge
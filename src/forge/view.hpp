#pragma once
#include <functional>
#include <iostream>
#include <tuple>

#include "storage.hpp"
namespace forge {
namespace _INTERNAL::ALGORITHMS {
template <typename PoolType>
static std::size_t get_driving_index(PoolType p) {
    std::size_t idx = 0;
    std::apply([&idx](auto&&... args) {
        std::size_t curr_idx = 0;
        std::size_t min = std::numeric_limits<std::size_t>::max();
        ([&idx, &curr_idx, &min](const auto& arg) {
            if (arg.size() > 0 && arg.size() < min) {
                min = arg.size();
                idx = curr_idx;
            }
            curr_idx++;
        }(args),
         ...);
    },
               p);
    return idx;
};

template <typename E, typename PoolType>
static bool contains_in_every(const E e, const PoolType p) {
    return std::apply([&e](auto&&... args) {
        return (args.contains(e) && ...);
    },
                      p);
};

template <std::size_t I, typename E, typename... Ts, typename PoolType>
static decltype(auto) pool_of(const E e, PoolType pool) {
    auto& storage = std::get<I>(pool);
    using query_type = std::tuple_element_t<I, std::tuple<Ts...>>;
    if constexpr (std::is_const_v<std::remove_reference_t<query_type>>) {
        return std::as_const(storage.get(e));
    } else {
        return (storage.get(e));
    }
};

}  // namespace _INTERNAL::ALGORITHMS
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
    basic_view_iterator& operator++(int) {
        ++this->ptr;
        return *this;
    }

    reference operator*() const { return *ptr; }
    pointer operator->() { return ptr; }
};

template <typename BaseIterator, typename PoolType, typename Ty, typename... Ts>
class view_iterator {
    using iterator_traits = std::iterator_traits<BaseIterator>;
    using pointer = iterator_traits::pointer;
    using value_type = iterator_traits::value_type;
    BaseIterator iter{nullptr};
    PoolType pool;

   public:
    view_iterator(const std::size_t driving_index, const std::size_t row_index, const PoolType pool)
        : pool{pool} {
        meta::_INTERNAL::homogeneous_template_tuple_get(driving_index, pool, [this, &row_index](auto&& arg) {
            this->iter = BaseIterator{std::addressof(*(arg.begin() + row_index))};
        });
    };
    view_iterator& operator++() {
        this->iter++;
        while (!_INTERNAL::ALGORITHMS::contains_in_every(*iter, this->pool)) {
            this->iter++;
        }
        return *this;
    }
    view_iterator& operator++(int) {
        ++this->iter;
        while (!_INTERNAL::ALGORITHMS::contains_in_every(*iter, this->pool)) {
            ++this->iter;
        }
        return *this;
    }

    decltype(auto) operator*() {
        return [this]<std::size_t... Is>(std::index_sequence<Is...>) {
            return std::forward_as_tuple(_INTERNAL::ALGORITHMS::pool_of<Is, Ty, Ts...>(*this->iter, this->pool)...);
        }(std::make_index_sequence<sizeof...(Ts)>{});
    }

    friend bool operator==(const view_iterator& a, const view_iterator& b) {
        return a.iter == b.iter;
    }
    friend bool operator!=(const view_iterator& a, const view_iterator& b) {
        return !(a == b);
    }
};

template <typename Ty, typename... Ts>
class basic_view_container {
   protected:
    using value_type = Ty;
    using pool_type = std::tuple<forge::storage::sparse_set<std::remove_cvref_t<Ts>, Ty>&...>;
    using iterator = view_iterator<basic_view_iterator<Ty>, pool_type, Ty, Ts...>;

   private:
    pool_type pool;
    std::size_t driving_index;

   public:
    basic_view_container(pool_type pool)
        : pool{pool},
          driving_index{_INTERNAL::ALGORITHMS::get_driving_index(pool)} {};

    iterator begin() {
        return iterator{this->driving_index, 0, this->pool};
    };

    iterator end() {
        std::size_t end_index = 0;
        meta::_INTERNAL::homogeneous_template_tuple_get(this->driving_index, this->pool, [&end_index](auto&& arg) {
            end_index = arg.size();
        });
        return iterator{this->driving_index, end_index, this->pool};
    };
};

template <typename Ty, typename... Ts>
class view_span : private basic_view_container<Ty, Ts...> {
    using container_traits = basic_view_container<Ty, Ts...>;
    using pool_type = container_traits::pool_type;
    using iterator = container_traits::iterator;

   public:
    view_span(pool_type pool) : basic_view_container<Ty, Ts...>{pool} {};

    container_traits& each() {
        return *this;
    };


    template <typename Func>
    void each(Func&& f) {
        for (const auto it = container_traits::begin(); it != container_traits::end(); it++) {
            std::forward<Func>(f)(*it);
        }
    };
};

};  // namespace forge
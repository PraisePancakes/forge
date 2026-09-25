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

    reference operator*() const { return *ptr; }
    pointer operator->() { return ptr; }
};

};  // namespace forge
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
    using pool_reference_type = std::tuple<forge::storage::sparse_set<Queries, Ty>&...>;

   private:
    pool_reference_type packed_storage_pool;

   public:
    view_span(pool_reference_type pool) : packed_storage_pool(pool) {};

    template <typename Func>
    void each(Func&& callback) {

    };
};
};  // namespace forge
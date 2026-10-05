#include <forge/forge.hpp>
#include <string>
#include <tuple>
#include <vector>

#include "doctest.h"

namespace {

struct A {};
struct B {};
struct C {};

}  // namespace

TEST_SUITE("meta") {
    TEST_CASE("contains_it") {
        CHECK(forge::meta::contains_it<int, char, int, float>);
        CHECK(forge::meta::contains_it<char, char, int, float>);
        CHECK(forge::meta::contains_it<float, char, int, float>);

        CHECK_FALSE(forge::meta::contains_it<double, char, int, float>);
        CHECK_FALSE(forge::meta::contains_it<int, char, float>);
    }

    TEST_CASE("index_of") {
        using tuple = std::tuple<int, char, float, double>;

        CHECK(forge::meta::index_of<int, tuple>::value == 0);
        CHECK(forge::meta::index_of<char, tuple>::value == 1);
        CHECK(forge::meta::index_of<float, tuple>::value == 2);
        CHECK(forge::meta::index_of<double, tuple>::value == 3);
    }

    TEST_CASE("index_of_value_type") {
        using tuple = std::tuple<std::vector<int>, std::vector<char>, std::vector<float>>;

        CHECK(forge::meta::index_of_value_type<int, tuple>::value == 0);
        CHECK(forge::meta::index_of_value_type<char, tuple>::value == 1);
        CHECK(forge::meta::index_of_value_type<float, tuple>::value == 2);
    }

    TEST_CASE("index_of_value_type works with different containers") {
        using tuple = std::tuple<std::vector<int>,
                                 std::basic_string<char>,
                                 std::vector<float>>;

        CHECK(forge::meta::index_of_value_type<int, tuple>::value == 0);
        CHECK(forge::meta::index_of_value_type<char, tuple>::value == 1);
        CHECK(forge::meta::index_of_value_type<float, tuple>::value == 2);
    }

    TEST_CASE("type_of") {
        CHECK(std::is_same_v<forge::meta::type_of_t<0, int, char, float>, int>);
        CHECK(std::is_same_v<forge::meta::type_of_t<1, int, char, float>, char>);
        CHECK(std::is_same_v<forge::meta::type_of_t<2, int, char, float>, float>);
    }

    TEST_CASE("type_of preserves cv and reference qualifiers") {
        CHECK(std::is_same_v<forge::meta::type_of_t<0, const int, char>, const int>);
        CHECK(std::is_same_v<forge::meta::type_of_t<1, int&, const char&>, const char&>);
    }

    TEST_CASE("is_unique_set") {
        CHECK(forge::meta::is_unique_set_v<int>);
        CHECK(forge::meta::is_unique_set_v<int, char>);
        CHECK(forge::meta::is_unique_set_v<int, char, float>);

        CHECK_FALSE(forge::meta::is_unique_set_v<int, int>);
        CHECK_FALSE(forge::meta::is_unique_set_v<int, char, int>);
        CHECK_FALSE(forge::meta::is_unique_set_v<int, char, float, char>);
    }

    TEST_CASE("is_unique_set supports template containers") {
        CHECK(forge::meta::is_unique_set_v<std::tuple<int, char, float>>);

        CHECK_FALSE(forge::meta::is_unique_set_v<std::tuple<int, char, int>>);

        CHECK(forge::meta::is_unique_set_v<std::tuple<std::vector<int>,
                                                      std::vector<char>,
                                                      std::vector<float>>>);
    }

    TEST_CASE("value") {
        using zero = forge::meta::value<0>;
        using three = forge::meta::value<3>;
        CHECK(zero::value == 0);
        CHECK(three::value == 3);
        CHECK(std::is_same_v<zero, std::integral_constant<std::size_t, 0>>);
    }

    TEST_CASE("homogeneous_template_tuple_get gets selected element") {
        std::tuple<int, int, int> tup{10, 20, 30};

        int result = 0;

        forge::meta::_INTERNAL::homogeneous_template_tuple_get(0, tup, [&](int value) {
            result = value;
        });

        CHECK(result == 10);

        forge::meta::_INTERNAL::homogeneous_template_tuple_get(1, tup, [&](int value) {
            result = value;
        });

        CHECK(result == 20);

        forge::meta::_INTERNAL::homogeneous_template_tuple_get(2, tup, [&](int value) {
            result = value;
        });

        CHECK(result == 30);
    }

    TEST_CASE("homogeneous_template_tuple_get only invokes callback once") {
        std::tuple<int, int, int> tup{10, 20, 30};

        int call_count = 0;
        int result = 0;

        forge::meta::_INTERNAL::homogeneous_template_tuple_get(1, tup, [&](int value) {
            ++call_count;
            result = value;
        });

        CHECK(call_count == 1);
        CHECK(result == 20);
    }

    TEST_CASE("homogeneous_template_tuple_get supports const tuple") {
        const std::tuple<int, int, int> tup{10, 20, 30};

        int result = 0;

        forge::meta::_INTERNAL::homogeneous_template_tuple_get(2, tup, [&](const int& value) {
            result = value;
        });

        CHECK(result == 30);
    }

    TEST_CASE("homogeneous_template_tuple_get can modify tuple element") {
        std::tuple<int, int, int> tup{10, 20, 30};

        forge::meta::_INTERNAL::homogeneous_template_tuple_get(1, tup, [&](int& value) {
            value = 42;
        });

        CHECK(std::get<0>(tup) == 10);
        CHECK(std::get<1>(tup) == 42);
        CHECK(std::get<2>(tup) == 30);
    }

    TEST_CASE("homogeneous_template_tuple_get works with heterogeneous tuple types sharing interface") {
        std::tuple<std::vector<int>,
                   std::vector<char>,
                   std::vector<float>>
            pools{
                std::vector<int>{1, 2, 3},
                std::vector<char>{'a', 'b'},
                std::vector<float>{4.0f, 5.0f, 6.0f}};

        std::size_t result = 0;

        forge::meta::_INTERNAL::homogeneous_template_tuple_get(0, pools, [&](auto& pool) {
            result = pool.size();
        });

        CHECK(result == 3);

        forge::meta::_INTERNAL::homogeneous_template_tuple_get(1, pools, [&](auto& pool) {
            result = pool.size();
        });

        CHECK(result == 2);

        forge::meta::_INTERNAL::homogeneous_template_tuple_get(2, pools, [&](auto& pool) {
            result = pool.size();
        });

        CHECK(result == 3);
    }

}  // TEST_SUITE
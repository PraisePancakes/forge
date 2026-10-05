#include <forge/forge.hpp>
#include <iostream>

#include "doctest.h"

TEST_SUITE("view iterator") {
    using namespace forge;

    using entity_traits_t = forge::entity_traits<forge::entity>;

    constexpr auto make_entity(
        entity_traits_t::entity_type id,
        entity_traits_t::version_type version = 0) noexcept {
        return entity_traits_t::construct(id, version);
    }

    TEST_CASE("view_iterator: single inclusion pool") {
        using pool = forge::storage::pool_storage<forge::entity, int>;
        using iterator = typename pool::iterator;

        pool ints;

        const auto e0 = make_entity(0);
        const auto e1 = make_entity(1);
        const auto e2 = make_entity(2);

        ints.emplace(e0, 0);
        ints.emplace(e1, 1);
        ints.emplace(e2, 2);

        using view_iterator_t = forge::view_iterator<
            iterator,
            forge::_INTERNAL::TAGS::deref_row_wise_tag,
            std::tuple<int>,
            std::tuple<>>;

        std::tuple<pool&> gets{ints};
        std::tuple<> excludes{};

        view_iterator_t it{0, gets, excludes};

        CHECK(*it == e0);

        ++it;
        CHECK(*it == e1);

        ++it;
        CHECK(*it == e2);

        ++it;
        CHECK(it == view_iterator_t{ints.size(), gets, excludes});
    }

    TEST_CASE("view_iterator: post increment") {
        using pool = forge::storage::pool_storage<forge::entity, int>;
        using iterator = typename pool::iterator;

        pool ints;

        const auto e0 = make_entity(0);
        const auto e1 = make_entity(1);
        const auto e2 = make_entity(2);

        ints.emplace(e0, 0);
        ints.emplace(e1, 1);
        ints.emplace(e2, 2);

        using view_iterator_t = forge::view_iterator<
            iterator,
            forge::_INTERNAL::TAGS::deref_row_wise_tag,
            std::tuple<int>,
            std::tuple<>>;

        std::tuple<pool&> gets{ints};
        std::tuple<> excludes{};

        view_iterator_t it{0, gets, excludes};

        auto old = it++;

        CHECK(*old == e0);
        CHECK(*it == e1);

        old = it++;

        CHECK(*old == e1);
        CHECK(*it == e2);

        old = it++;

        CHECK(*old == e2);
        CHECK(it == view_iterator_t{ints.size(), gets, excludes});
    }

    TEST_CASE("view_iterator: equality") {
        using pool = forge::storage::pool_storage<forge::entity, int>;
        using iterator = typename pool::iterator;

        pool ints;

        const auto e0 = make_entity(0);
        const auto e1 = make_entity(1);

        ints.emplace(e0, 0);
        ints.emplace(e1, 1);

        using view_iterator_t = forge::view_iterator<
            iterator,
            forge::_INTERNAL::TAGS::deref_row_wise_tag,
            std::tuple<int>,
            std::tuple<>>;

        std::tuple<pool&> gets{ints};
        std::tuple<> excludes{};

        view_iterator_t a{0, gets, excludes};
        view_iterator_t b{0, gets, excludes};
        view_iterator_t c{1, gets, excludes};

        CHECK(a == b);
        CHECK_FALSE(a != b);

        CHECK(a != c);
        CHECK_FALSE(a == c);
    }

    TEST_CASE("pool iterator: multiple entities") {
        using pool = forge::storage::pool_storage<forge::entity, int>;
        using iterator = typename pool::iterator;

        pool ints;

        const auto e0 = make_entity(0);
        const auto e1 = make_entity(1);
        const auto e2 = make_entity(2);

        ints.emplace(e0, 0);
        ints.emplace(e1, 1);
        ints.emplace(e2, 2);

        auto it = ints.begin();

        CHECK(*it == e0);

        ++it;
        CHECK(*it == e1);

        ++it;
        CHECK(*it == e2);

        ++it;
        CHECK(it == ints.end());
    }

    TEST_CASE("view_iterator: multiple inclusion pools") {
        using int_pool = forge::storage::pool_storage<forge::entity, int>;
        using char_pool = forge::storage::pool_storage<forge::entity, char>;

        using iterator = typename int_pool::iterator;

        int_pool ints;
        char_pool chars;

        const auto e0 = make_entity(0);
        const auto e1 = make_entity(1);
        const auto e2 = make_entity(2);

        ints.emplace(e0, 0);
        ints.emplace(e1, 1);
        ints.emplace(e2, 2);

        chars.emplace(e0, 'a');
        chars.emplace(e1, 'b');

        using view_iterator_t = forge::view_iterator<
            iterator,
            forge::_INTERNAL::TAGS::deref_row_wise_tag,
            std::tuple<int, char>,
            std::tuple<>>;

        std::tuple<int_pool&, char_pool&> gets{ints, chars};
        std::tuple<> excludes{};

        view_iterator_t it{0, gets, excludes};

        CHECK(*it == e0);

        ++it;
        CHECK(*it == e1);
    }

    TEST_CASE("view_iterator: second inclusion pool as driving pool") {
        using int_pool = forge::storage::pool_storage<forge::entity, int>;
        using char_pool = forge::storage::pool_storage<forge::entity, char>;

        using iterator = typename char_pool::iterator;

        int_pool ints;
        char_pool chars;

        const auto e0 = make_entity(0);
        const auto e1 = make_entity(1);
        const auto e2 = make_entity(2);

        ints.emplace(e0, 0);
        ints.emplace(e1, 1);

        chars.emplace(e0, 'a');
        chars.emplace(e1, 'b');
        chars.emplace(e2, 'c');

        using view_iterator_t = forge::view_iterator<
            iterator,
            forge::_INTERNAL::TAGS::deref_row_wise_tag,
            std::tuple<int, char>,
            std::tuple<>>;

        std::tuple<int_pool&, char_pool&> gets{ints, chars};
        std::tuple<> excludes{};

        view_iterator_t it{0, gets, excludes};

        CHECK(*it == e0);

        ++it;
        CHECK(*it == e1);

        ++it;
        CHECK(it == view_iterator_t{ints.size(), gets, excludes});
    }

    TEST_CASE("view_iterator: exclusion") {
        using int_pool = forge::storage::pool_storage<forge::entity, int>;
        using char_pool = forge::storage::pool_storage<forge::entity, char>;

        using iterator = typename int_pool::iterator;

        int_pool ints;
        char_pool chars;

        const auto e0 = make_entity(0);
        const auto e1 = make_entity(1);
        const auto e2 = make_entity(2);

        ints.emplace(e0, 0);
        ints.emplace(e1, 1);
        ints.emplace(e2, 2);

        chars.emplace(e1, 'x');

        using view_iterator_t = forge::view_iterator<
            iterator,
            forge::_INTERNAL::TAGS::deref_row_wise_tag,
            std::tuple<int>,
            std::tuple<char>>;

        std::tuple<int_pool&> gets{ints};
        std::tuple<char_pool&> excludes{chars};

        view_iterator_t it{0, gets, excludes};

        CHECK(*it == e0);

        ++it;
        CHECK(*it == e2);

        ++it;
        CHECK(it == view_iterator_t{ints.size(), gets, excludes});
    }

    TEST_CASE("view_iterator: multiple exclusions") {
        using int_pool = forge::storage::pool_storage<forge::entity, int>;
        using char_pool = forge::storage::pool_storage<forge::entity, char>;
        using float_pool = forge::storage::pool_storage<forge::entity, float>;

        using iterator = typename int_pool::iterator;

        int_pool ints;
        char_pool chars;
        float_pool floats;

        const auto e0 = make_entity(0);
        const auto e1 = make_entity(1);
        const auto e2 = make_entity(2);
        const auto e3 = make_entity(3);

        ints.emplace(e0, 0);
        ints.emplace(e1, 1);
        ints.emplace(e2, 2);
        ints.emplace(e3, 3);

        chars.emplace(e1, 'x');
        floats.emplace(e2, 2.0f);

        using view_iterator_t = forge::view_iterator<
            iterator,
            forge::_INTERNAL::TAGS::deref_row_wise_tag,
            std::tuple<int>,
            std::tuple<char, float>>;

        std::tuple<int_pool&> gets{ints};
        std::tuple<char_pool&, float_pool&> excludes{chars, floats};

        view_iterator_t it{0, gets, excludes};

        CHECK(*it == e0);

        ++it;
        CHECK(*it == e3);

        ++it;
        CHECK(it == view_iterator_t{ints.size(), gets, excludes});
    }

    TEST_CASE("view_iterator: column-wise dereference") {
        using int_pool = forge::storage::pool_storage<forge::entity, int>;
        using char_pool = forge::storage::pool_storage<forge::entity, char>;

        using iterator = typename int_pool::iterator;

        int_pool ints;
        char_pool chars;

        const auto e0 = make_entity(0);
        const auto e1 = make_entity(1);

        ints.emplace(e0, 10);
        ints.emplace(e1, 20);

        chars.emplace(e0, 'a');
        chars.emplace(e1, 'b');

        using view_iterator_t = forge::view_iterator<
            iterator,
            forge::_INTERNAL::TAGS::deref_column_wise_tag,
            std::tuple<int, char>,
            std::tuple<>>;

        std::tuple<int_pool&, char_pool&> gets{ints, chars};
        std::tuple<> excludes{};

        view_iterator_t it{0, gets, excludes};

        auto [i0, c0] = *it;

        CHECK(i0 == 10);
        CHECK(c0 == 'a');

        ++it;

        auto [i1, c1] = *it;

        CHECK(i1 == 20);
        CHECK(c1 == 'b');
    }

    TEST_CASE("view_iterator: column-wise dereference returns references") {
        using int_pool = forge::storage::pool_storage<forge::entity, int>;
        using char_pool = forge::storage::pool_storage<forge::entity, char>;

        using iterator = typename int_pool::iterator;

        int_pool ints;
        char_pool chars;

        const auto e0 = make_entity(0);

        ints.emplace(e0, 10);
        chars.emplace(e0, 'a');

        using view_iterator_t = forge::view_iterator<
            iterator,
            forge::_INTERNAL::TAGS::deref_column_wise_tag,
            std::tuple<int, char>,
            std::tuple<>>;

        std::tuple<int_pool&, char_pool&> gets{ints, chars};
        std::tuple<> excludes{};

        view_iterator_t it{0, gets, excludes};

        auto&& [i, c] = *it;

        static_assert(std::is_lvalue_reference_v<decltype(i)>);
        static_assert(std::is_lvalue_reference_v<decltype(c)>);

        CHECK(&i == &ints.get(e0));
        CHECK(&c == &chars.get(e0));

        i = 42;
        c = 'z';

        CHECK(ints.get(e0) == 42);
        CHECK(chars.get(e0) == 'z');
    }

    TEST_CASE("view_iterator: empty driving pool") {
        using int_pool = forge::storage::pool_storage<forge::entity, int>;
        using char_pool = forge::storage::pool_storage<forge::entity, char>;

        using iterator = typename int_pool::iterator;

        int_pool ints;
        char_pool chars;

        const auto e0 = make_entity(0);

        chars.emplace(e0, 'a');

        using view_iterator_t = forge::view_iterator<
            iterator,
            forge::_INTERNAL::TAGS::deref_row_wise_tag,
            std::tuple<int, char>,
            std::tuple<>>;

        std::tuple<int_pool&, char_pool&> gets{ints, chars};
        std::tuple<> excludes{};

        view_iterator_t begin{0, gets, excludes};
        view_iterator_t end{ints.size(), gets, excludes};

        CHECK(begin == end);
    }
}

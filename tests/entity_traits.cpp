#include <forge/forge.hpp>
#include <iostream>

#include "doctest.h"

TEST_SUITE("entity traits") {
    using u64_t = forge::basic_entity_traits<std::uint64_t>;
    using u32_t = forge::basic_entity_traits<std::uint32_t>;
    TEST_CASE("entity representation") {
        CHECK(u64_t::partitioned_length == 32);
        CHECK(u32_t::partitioned_length == 16);
        auto constructed_u64 = u64_t::construct(10, 5);
        CHECK(u64_t::to_bit_string(constructed_u64) == "00000000000000000000000000001010 00000000000000000000000000000101");
        auto constructed_u32 = u32_t::construct(1, 2);
        CHECK(u32_t::to_bit_string(constructed_u32) == "0000000000000001 0000000000000010");
        CHECK(u64_t::to_entity(constructed_u64) == 10);
        CHECK(u32_t::to_entity(constructed_u32) == 1);

        CHECK(u32_t::to_version(constructed_u32) == 2);
        CHECK(u64_t::to_version(constructed_u64) == 5);

        CHECK(u32_t::to_version(u32_t::next(constructed_u32)) == 3);
        CHECK(u64_t::to_version(u64_t::next(constructed_u64)) == 6);

        std::uint64_t null_64 = forge::null;
        std::uint32_t null_32 = forge::null;
        CHECK(u64_t::to_bit_string(null_64) == "11111111111111111111111111111111 11111111111111111111111111111111");
        CHECK(u32_t::to_bit_string(null_32) == "1111111111111111 1111111111111111");
    };

    TEST_CASE("entity forwarding") {
        auto constructed_u64 = u64_t::construct(100, 50);
        CHECK(forge::to_entity(constructed_u64) == 100);
        CHECK(forge::to_version(constructed_u64) == 50);
        CHECK(forge::to_value(constructed_u64) == constructed_u64);
    };
};
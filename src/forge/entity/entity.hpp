#pragma once
#include <bit>
#include <cstdint>
namespace forge {

template <typename>
struct entity_traits;

template <typename T>
    requires requires { typename T::entity_type; }
struct entity_traits<T> {
    using entity_type = T::entity_type;
};

template <>
struct entity_traits<std::uint64_t> {
    using value_type = std::uint64_t;
    using entity_type = std::uint32_t;
    using version_type = std::uint32_t;
};

template <>
struct entity_traits<std::uint32_t> {
    using value_type = std::uint32_t;
    using entity_type = std::uint16_t;
    using version_type = std::uint16_t;
};

template <typename E>
struct basic_entity_traits {
    using Traits = entity_traits<E>;
    using value_type = Traits::value_type;
    using version_type = Traits::version_type;
    using entity_type = Traits::entity_type;
    // can be length of either version or entity
    static constexpr auto length = std::bit_width(version_type{});
    static_assert(length == std::bit_width(entity_type{}) && length * 2 == std::bit_width(value_type{}), "identifier types must be bit-symmetric and half of value_type");
};

};  // namespace forge
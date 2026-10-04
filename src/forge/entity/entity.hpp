#pragma once
#include <bit>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
namespace forge {
namespace internal {
template <typename>
struct entity_traits;

template <typename T>
    requires requires { typename T::entity_type; }
struct entity_traits<T> {
    using value_type = T;
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

}  // namespace internal
template <typename Type>
concept entity_like = requires {
    typename internal::entity_traits<Type>::value_type;
};
template <typename E>
struct basic_entity_traits {
    using Traits = internal::entity_traits<E>;
    using value_type = Traits::value_type;
    using version_type = Traits::version_type;
    using entity_type = Traits::entity_type;
    // can be length of either version or entity
    static constexpr auto partitioned_length = std::numeric_limits<version_type>::digits;
    static constexpr auto length = std::numeric_limits<value_type>::digits;
    static_assert(partitioned_length == std::numeric_limits<entity_type>::digits && partitioned_length * 2 == length, "identifier types must be bit-symmetric and half of value_type");

    template <typename V>
        requires std::same_as<std::remove_cvref_t<V>, value_type>
    [[nodiscard]] static constexpr auto to_value(const V value) noexcept {
        return static_cast<value_type>(value);
    }

    [[nodiscard]] static constexpr auto construct(const entity_type e, const version_type v) noexcept {
        return static_cast<value_type>(static_cast<value_type>(e) << partitioned_length) | static_cast<value_type>(v);
    };
    template <typename V>
        requires std::same_as<std::remove_cvref_t<V>, value_type>
    [[nodiscard]] static constexpr auto to_entity(const V v) noexcept {
        return static_cast<entity_type>(v >> partitioned_length);
    }

    template <typename V>
        requires std::same_as<std::remove_cvref_t<V>, value_type>
    [[nodiscard]] static constexpr auto to_version(const V v) noexcept {
        return static_cast<version_type>(v);
    }

    template <typename V>
        requires std::same_as<std::remove_cvref_t<V>, value_type>
    [[nodiscard]] static constexpr auto next(const V v) noexcept {
        const version_type next_version = to_version(v) + 1;
        return construct(to_entity(v), next_version);
    }

    template <typename V>
        requires std::same_as<std::remove_cvref_t<V>, value_type>
    static std::string to_bit_string(const V value) noexcept {
        std::bitset<length> b{value};
        std::string s{b.to_string()};
        std::string l = s.substr(0, partitioned_length);
        std::string r = s.substr(partitioned_length, partitioned_length);
        assert(l.size() == r.size());
        return l + " " + r;
    }
};

template <entity_like T>
struct entity_traits : basic_entity_traits<internal::entity_traits<T>> {
    using base_type = basic_entity_traits<internal::entity_traits<T>>;
};
template <typename Entity>
[[nodiscard]] constexpr basic_entity_traits<Entity>::value_type to_value(const Entity value) noexcept {
    return basic_entity_traits<Entity>::to_value(value);
}

template <typename Entity>
[[nodiscard]] constexpr basic_entity_traits<Entity>::entity_type to_entity(const Entity value) noexcept {
    return basic_entity_traits<Entity>::to_entity(value);
}

template <typename Entity>
[[nodiscard]] constexpr basic_entity_traits<Entity>::version_type to_version(const Entity value) noexcept {
    return basic_entity_traits<Entity>::to_version(value);
}
struct null_t {
    template <entity_like Entity>
    [[nodiscard]] constexpr operator Entity() const noexcept {
        using traits_type = basic_entity_traits<Entity>;
        return traits_type::construct(std::numeric_limits<typename traits_type::entity_type>::max(), std::numeric_limits<typename traits_type::version_type>::max());
    }

    [[nodiscard]] bool operator==(null_t) noexcept {
        return true;
    }
    [[nodiscard]] bool operator!=(null_t) noexcept {
        return false;
    }
};

inline constexpr static null_t null{};

};  // namespace forge
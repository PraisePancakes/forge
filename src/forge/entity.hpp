#pragma once
#include <bitset>
#include <cassert>
#include <cstdint>
#include <limits>
#include <ostream>
#include <string>
#include <type_traits>
namespace forge {

template <typename PackedTy>
    requires(std::is_unsigned_v<PackedTy>)
struct entity_representation {
    using value_type = PackedTy;
};

template <>
struct entity_representation<std::uint32_t> {
    using value_type = std::uint32_t;
    using id_type = std::uint16_t;
    using version_type = std::uint16_t;
};

template <>
struct entity_representation<std::uint64_t> {
    using value_type = std::uint64_t;
    using id_type = std::uint32_t;
    using version_type = std::uint32_t;
};

//                                                    id  ver
// value type is the packed entity representation => 0001 0000, where higher bits are the id and lower bits are the version

namespace _INTERNAL {
template <typename EntityTp>
struct base_entity {
    using traits = entity_representation<EntityTp>;
    using value_type = typename traits::value_type;
    using id_type = typename traits::id_type;
    using version_type = typename traits::version_type;

   public:
    static value_type generate_next() noexcept {
        static id_type serial{0};
        return static_cast<value_type>(serial++) << std::numeric_limits<version_type>::digits;
    };

    [[nodiscard]] static constexpr id_type to_id(value_type value) noexcept {
        return static_cast<id_type>(value >> std::numeric_limits<version_type>::digits);
    };

    [[nodiscard]] static constexpr version_type to_version(value_type value) noexcept {
        return static_cast<version_type>(value);
    };

    [[nodiscard]] static constexpr value_type next(value_type value) noexcept {
        version_type new_version = to_version(value) + 1;
        id_type id = to_id(value);
        return (static_cast<value_type>(id) << std::numeric_limits<id_type>::digits) | new_version;
    };

    static std::string to_bit_string(value_type value) noexcept {
        std::bitset<std::numeric_limits<value_type>::digits> b{value};
        std::string s{b.to_string()};
        std::string l = s.substr(0, std::numeric_limits<id_type>::digits);
        std::string r = s.substr(std::numeric_limits<id_type>::digits, std::numeric_limits<version_type>::digits);
        assert(l.size() == r.size());
        return l + " " + r;
    };
};
}  // namespace _INTERNAL

template <typename EntityType>
struct entity_fwd : _INTERNAL::base_entity<EntityType> {
    using base_type = _INTERNAL::base_entity<EntityType>;
    typename base_type::value_type value;

    operator typename base_type::value_type() const {
        return this->value;
    };
    entity_fwd() : value{base_type::generate_next()} {};

    explicit constexpr entity_fwd(typename base_type::value_type value)
        : value{value} {}

    friend std::ostream& operator<<(std::ostream& os, const entity_fwd<EntityType>& e) {
        os << "Entity {" << std::endl;
        os << "BIT REP : " + base_type::to_bit_string(e) << std::endl;
        os << "ID      : " + std::to_string(base_type::to_id(e)) << std::endl;
        os << "VERSION : " + std::to_string(base_type::to_version(e)) << std::endl;
        os << "};" << std::endl;
        return os;
    };
};

template <typename T>
static constexpr typename entity_fwd<T>::base_type::id_type to_id(entity_fwd<T> e) {
    return entity_fwd<T>::base_type::to_id(e);
};

template <typename T>
static constexpr typename entity_fwd<T>::base_type::version_type to_version(entity_fwd<T> e) {
    return entity_fwd<T>::base_type::to_version(e);
};

template <typename T>
static constexpr entity_fwd<T> next(entity_fwd<T> e) {
    return entity_fwd<T>{entity_fwd<T>::base_type::next(e)};
}

};  // namespace forge
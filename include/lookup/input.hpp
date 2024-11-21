#pragma once
#include <lookup/entry.hpp>

#include <stdx/compiler.hpp>
#include <stdx/concepts.hpp>
#include <stdx/type_traits.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace lookup {
template <typename K, typename V = K, std::size_t N = 0> struct input {
    using key_type = K;
    using value_type = V;

    using array_t = std::array<entry<K, V>, N>;
    constexpr static auto size = std::integral_constant<std::size_t, N>{};

    constexpr input() = default;
    constexpr explicit input(value_type const &v) : default_value{v} {}
    constexpr input(value_type const &v, array_t const &a)
        : default_value{v}, entries{a} {}

    V default_value{};
    std::array<entry<K, V>, N> entries{};
};

template <typename V, typename A>
input(V, A) -> input<typename A::value_type::key_type,
                     typename A::value_type::value_type, std::size(A{})>;

template <typename V> input(V) -> input<V>;

template <typename T>
concept compile_time = stdx::is_cx_value_v<T>;
} // namespace lookup

#ifndef BASE_CONCEPTS_HPP
#define BASE_CONCEPTS_HPP

#include <ranges>
#include <concepts>

namespace hana {

template <auto Value> struct fixed { };
template <auto Value> constexpr auto fixed_cast = fixed<Value>{};

template <typename T, typename... Types> concept one_of = (std::same_as<T, Types> || ...);

template <typename T> concept byte = one_of<std::remove_cvref_t<T>, char, unsigned char, signed char, uint8_t, int8_t, std::byte>;

template <typename Encoding> concept padded_encoding = requires() {
	{ Encoding::padding } -> hana::byte;
};

template <typename T> concept byte_range = requires() {
	requires std::ranges::range<T>;
	requires hana::byte<std::ranges::range_value_t<T>>;
};

} // namespace hana

#endif

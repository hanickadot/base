#ifndef HANA_CHUNK_OF_BITS_HPP
#define HANA_CHUNK_OF_BITS_HPP

#include "concepts.hpp"
#include <bit>
#include <numeric>
#include <ranges>
#include <concepts>

namespace hana {

template <size_t Bits> struct select_bit_integer;

template <size_t Bits> requires(Bits <= 8) struct select_bit_integer<Bits> {
	using type = uint8_t;
};

template <size_t Bits> requires(Bits > 8 && Bits <= 16) struct select_bit_integer<Bits> {
	using type = uint16_t;
};

template <size_t Bits> requires(Bits > 16 && Bits <= 32) struct select_bit_integer<Bits> {
	using type = uint32_t;
};

template <size_t Bits> requires(Bits > 32 && Bits <= 64) struct select_bit_integer<Bits> {
	using type = uint64_t;
};

template <size_t Bits> using select_bit_integer_t = select_bit_integer<Bits>::type;

template <size_t V> requires(std::popcount(V) == 1u) constexpr auto log2_of_power_of_2 = static_cast<size_t>(std::countr_zero(V));

template <size_t Bits> constexpr auto mask = static_cast<select_bit_integer_t<Bits>>((static_cast<select_bit_integer_t<Bits>>(1u) << Bits) - 1u);

template <size_t Capacity> class basic_bit_buffer {
public:
	static constexpr auto capacity = std::integral_constant<size_t, Capacity>{};
	using storage_type = select_bit_integer_t<capacity()>;
	using size_type = select_bit_integer_t<log2_of_power_of_2<std::bit_ceil(capacity())>>;

private:
	storage_type buffer{0};
	size_type bits_available{0};

public:
	template <size_t Bits> static constexpr auto mask = static_cast<storage_type>((static_cast<storage_type>(1u) << Bits) - 1u);

	constexpr size_t size() const noexcept {
		return bits_available;
	}

	constexpr size_t unused_size() const noexcept {
		return capacity() - bits_available;
	}

	constexpr bool empty() const noexcept {
		return bits_available == 0u;
	}

	constexpr bool full() const noexcept {
		return bits_available == capacity();
	}

	template <size_t Bits> constexpr void push(select_bit_integer_t<Bits> in) noexcept requires(Bits <= capacity()) {
		assert(size() <= capacity() - Bits);
		buffer = (buffer << Bits) | in;
		bits_available += Bits;
	}
	template <size_t Bits> constexpr void pop() noexcept requires(Bits <= capacity()) {
		assert(size() >= Bits);
		bits_available -= Bits;
	}

	template <size_t Bits> constexpr auto front() const noexcept -> select_bit_integer_t<Bits> requires(Bits <= capacity()) {
		using output_type = select_bit_integer_t<Bits>;
		assert(size() >= Bits);
		return static_cast<output_type>((buffer >> (bits_available - Bits))) & mask<Bits>;
	}
};

template <size_t OutBits, size_t InBits = 8u> class bit_buffer: protected basic_bit_buffer<std::lcm(OutBits, InBits)> {
	using super = basic_bit_buffer<std::lcm(OutBits, InBits)>;

public:
	constexpr bit_buffer() noexcept = default;

	static constexpr auto out_bits = std::integral_constant<size_t, OutBits>{};
	static constexpr auto in_bits = std::integral_constant<size_t, InBits>{};

	using out_type = select_bit_integer_t<OutBits>;
	using in_type = select_bit_integer_t<InBits>;

	using super::capacity;
	static constexpr auto in_capacity = std::integral_constant<size_t, (capacity() / in_bits())>{};
	static constexpr auto out_capacity = std::integral_constant<size_t, (capacity() / out_bits())>{};

	constexpr void push(in_type in) noexcept {
		super::template push<in_bits>(in);
	}

	constexpr void push_empty() noexcept {
		this->push(in_type{0u});
	}

	constexpr void pop() noexcept {
		super::template pop<out_bits>();
	}

	constexpr auto front() const noexcept -> out_type {
		return super::template front<out_bits>();
	}

	constexpr auto size() const noexcept {
		return super::size() / out_bits;
	}

	constexpr auto unused_size() const noexcept {
		return super::unused_size() / in_bits;
	}

	using super::empty;
	using super::full;

	constexpr bool has_bits_for_pop() const noexcept {
		return super::size() >= out_bits;
	}

	constexpr bool has_capacity_for_push() const noexcept {
		return super::size() <= (capacity() - in_bits);
	}
};

} // namespace hana

#endif

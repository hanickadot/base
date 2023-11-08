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

constexpr size_t log2_of_power_of_2(size_t in) noexcept {
	// assert(std::popcount(in) == 1u);
	return static_cast<size_t>(std::countr_zero(in));
}

template <size_t Bits> constexpr auto mask = static_cast<select_bit_integer_t<Bits>>((static_cast<select_bit_integer_t<Bits>>(1u) << Bits) - 1u);

template <size_t Capacity> class basic_bit_buffer {
public:
	static constexpr auto capacity = std::integral_constant<size_t, Capacity>{};
	using storage_type = select_bit_integer_t<Capacity>;
	using buffer_size_type = select_bit_integer_t<log2_of_power_of_2(Capacity)>;
	static_assert(std::same_as<uint8_t, buffer_size_type>); // always (for now)

private:
	storage_type buffer{0};
	buffer_size_type bits_available{0};

	template <size_t Bits> static constexpr auto mask = static_cast<storage_type>((static_cast<storage_type>(1u) << Bits) - 1u);

public:
	constexpr size_t size() const noexcept {
		return bits_available;
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

template <size_t OutBits, size_t InBits = 8u> class bit_buffer: basic_bit_buffer<std::lcm(OutBits, InBits)> {
	using super = basic_bit_buffer<std::lcm(OutBits, InBits)>;

public:
	static constexpr auto out_bits = std::integral_constant<size_t, OutBits>{};
	static constexpr auto in_bits = std::integral_constant<size_t, InBits>{};

	using out_type = select_bit_integer_t<OutBits>;
	using in_type = select_bit_integer_t<InBits>;

	using super::capacity;

	template <size_t CustomInSize = InBits> static constexpr auto push_count = std::integral_constant<size_t, (capacity() / CustomInSize)>{};
	template <size_t CustomOutSize = OutBits> static constexpr auto pop_count = std::integral_constant<size_t, (capacity() / CustomOutSize)>{};

	using super::push;
	constexpr void push(in_type in) noexcept {
		super::template push<in_bits>(in);
	}

	using super::pop;
	constexpr void pop() noexcept {
		super::template pop<out_bits>();
	}

	using super::front;
	constexpr auto front() const noexcept -> out_type {
		return super::template front<out_bits>();
	}

	using super::empty;
	using super::full;
	using super::size;

	constexpr auto count() const noexcept {
		return size() / out_bits;
	}

	constexpr bool has_bits_for_pop() const noexcept {
		return size() >= out_bits;
	}

	constexpr bool has_capacity_for_push() const noexcept {
		return size() <= (capacity() - in_bits);
	}
};

// chunk_of_bits

template <size_t Bits, typename OutT, std::ranges::input_range Range> requires std::is_trivial_v<std::ranges::range_value_t<Range>> struct chunk_of_bits_view {
	static_assert(Bits <= 64);

	struct value_type {
		OutT value;
		uint8_t bits;
	};

	using size_type = size_t;

	Range range;

	struct iterator {
		using value_type = chunk_of_bits_view::value_type;
		using difference_type = ptrdiff_t;

		std::ranges::iterator_t<Range> it{};
		std::ranges::sentinel_t<Range> end{};

		mutable value_type output_buffer{};

		constexpr iterator(std::ranges::iterator_t<Range> _it, std::ranges::sentinel_t<Range> _end): it{_it}, end{_end} {
			operator++();
		}

		iterator() = default;
		iterator(const iterator &) = default;
		iterator(iterator &&) = default;
		~iterator() = default;

		iterator & operator=(const iterator &) = default;
		iterator & operator=(iterator &&) = default;

		constexpr iterator & operator++() noexcept {
			// fill buffer
			++it;
			return *this;
		}

		constexpr iterator operator++(int) noexcept {
			auto copy = *this;
			this->operator++();
			return copy;
		}

		constexpr const value_type & operator*() const noexcept {
			return output_buffer;
		}

		constexpr friend bool operator==(const iterator & lhs, const iterator & rhs) noexcept {
			assert(lhs.end == rhs.end);
			return lhs.it == rhs.it;
		}
	};

	static_assert(std::input_iterator<iterator>);

	struct sentinel {
		constexpr friend bool operator==(sentinel, const iterator & it) noexcept {
			return it.it == it.end;
		}
	};

	static_assert(std::sentinel_for<sentinel, iterator>);

	constexpr chunk_of_bits_view(Range _range) noexcept: range{std::move(_range)} { }

	constexpr auto begin() const noexcept {
		return iterator{range.begin(), range.end()};
	}

	constexpr auto end() const noexcept {
		return sentinel{};
	}

	constexpr size_type size() const noexcept requires(std::ranges::sized_range<Range>) {
		return (sizeof(std::ranges::range_value_t<Range>) * 8u * std::ranges::size(range) + (Bits - 1u)) / Bits;
	}
};

template <size_t Bits, typename OutT = select_bit_integer_t<Bits>> struct chunk_of_bits_action {
	template <std::ranges::input_range Range> constexpr auto operator()(Range && range) noexcept {
		return chunk_of_bits_view<Bits, OutT, Range>{std::move(range)};
	}

	template <std::ranges::input_range Range> friend constexpr auto operator|(Range && range, chunk_of_bits_action v) noexcept {
		return v.operator()(std::forward<Range>(range));
	}
};

template <size_t Bits, typename OutT = select_bit_integer_t<Bits>> constexpr auto chunk_of_bits = chunk_of_bits_action<Bits, OutT>{};

} // namespace hana

#endif

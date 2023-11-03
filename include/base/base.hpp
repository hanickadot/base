#ifndef BASE_BASE_HPP
#define BASE_BASE_HPP

#include "concepts.hpp"
#include "encodings.hpp"
#include <bit>
#include <iostream>
#include <numeric>
#include <ranges>
#include <string>
#include <cassert>
#include <concepts>
#include <cstddef>

namespace hana {

template <typename Encoding> struct encoding_properties {
	static_assert(Encoding::alphabet[std::size(Encoding::alphabet) - 1u] == '\0'); // must be zero-terminated

	static constexpr size_t size = std::size(Encoding::alphabet) - 1u;

	static_assert(std::popcount(size) == 1u); // must be 2^n

	static constexpr size_t bits = std::countr_zero(size);

	static_assert(bits <= 8);
	static_assert(bits > 0);

	static constexpr bool has_padding = padded_encoding<Encoding>;

	static constexpr char padding = [] {
		if constexpr (has_padding) {
			return Encoding::padding;
		} else {
			return '\0';
		}
	}();

	static constexpr size_t block_size_bits = std::lcm(bits, 8);
	static constexpr size_t input_block_size = block_size_bits / 8;
	static constexpr size_t output_block_size = block_size_bits / bits;

	static constexpr auto alphabet = [] {
		std::array<char, 256> output;

		std::fill(output.begin(), output.end(), padding);

		for (size_t i = 0; i != size; ++i) {
			output[i] = Encoding::alphabet[i];
		}

		return output;
	}();

	static consteval auto calculate_reverse_alphabet() {
		std::array<uint8_t, 256> output;

		std::fill(output.begin(), output.end(), '\0'); // error indicator?

		for (size_t i = 0; i != size; ++i) {
			output[static_cast<uint8_t>(Encoding::alphabet[i])] = static_cast<uint8_t>(i);
		}

		if constexpr (has_padding) {
			output[Encoding::padding] = '\0';
		}

		return output;
	}

	static constexpr auto reverse_alphabet = calculate_reverse_alphabet();

	static constexpr size_t calculate_size(size_t input) noexcept {
		return (input + (input_block_size - 1u)) / input_block_size * output_block_size;
	}
};

static_assert(encoding_properties<encoding::base64>::size == 64);
static_assert(encoding_properties<encoding::base64>::bits == 6);
static_assert(encoding_properties<encoding::base64>::input_block_size == 3);
static_assert(encoding_properties<encoding::base64>::output_block_size == 4);

static_assert(encoding_properties<encoding::base32>::size == 32);
static_assert(encoding_properties<encoding::base32>::bits == 5);
static_assert(encoding_properties<encoding::base32>::input_block_size == 5);
static_assert(encoding_properties<encoding::base32>::output_block_size == 8);

static_assert(encoding_properties<encoding::base16>::size == 16);
static_assert(encoding_properties<encoding::base16>::bits == 4);
static_assert(encoding_properties<encoding::base16>::input_block_size == 1);
static_assert(encoding_properties<encoding::base16>::output_block_size == 2);

static_assert(encoding_properties<encoding::base8>::size == 8);
static_assert(encoding_properties<encoding::base8>::bits == 3);
static_assert(encoding_properties<encoding::base8>::input_block_size == 3);
static_assert(encoding_properties<encoding::base8>::output_block_size == 8);

static_assert(encoding_properties<encoding::base4>::size == 4);
static_assert(encoding_properties<encoding::base4>::bits == 2);
static_assert(encoding_properties<encoding::base4>::input_block_size == 1);
static_assert(encoding_properties<encoding::base4>::output_block_size == 4);

static_assert(encoding_properties<encoding::base2>::size == 2);
static_assert(encoding_properties<encoding::base2>::bits == 1);
static_assert(encoding_properties<encoding::base2>::input_block_size == 1);
static_assert(encoding_properties<encoding::base2>::output_block_size == 8);

// [AAAA aaaa] [BBBB bbbb] [CCCC cccc] [DDDD dddd] [EEEE eeee] (base256 = byte)

// [AAAAaa] [aaBBBB] [bbbbCC] [CCcccc] (base64)
// [AAAAa] [aaaBB] [BBbbb] [bCCCC] [ccccD] [DDDdd] [ddEEE] [Eeeee] (base32)
// [AAAA] [aaaa] (base16)
// [AAA] [Aaa] [aaB] [BBB] [bbb] [bCC] [CCc] [ccc] (base8)
// [AA] [AA] [aa] [aa] (base4)

template <typename It, typename End> struct read_bits {
	It it;
	End end;

	static constexpr uint16_t padding_value = 0xF000u;
	static constexpr uint8_t mask = 0b00'111111u;

	uint16_t tmp{0};
	uint8_t counter{0};

	constexpr uint16_t read_byte(fixed<true> = {}) {
		if (it == end) {
			return padding_value;
		}

		const auto r = static_cast<uint8_t>(*it);
		++it;
		return r;
	}

	constexpr uint16_t read_byte(fixed<false>) {
		return 0u;
	}

	template <bool NeedRead, size_t OldShift, size_t NewShift> constexpr uint8_t read_part(uint16_t & previous) {
		const uint16_t byte = read_byte(fixed_cast<NeedRead>);

		const uint8_t r = (( // first and newly read byte
							   (previous << OldShift) | (byte >> NewShift))
							  & mask) // we observe only first 1-7 bits
			| (previous >> 8u);		  // if previous read failed we use highest bit to get padding mark

		previous = byte;

		return r;
	}

	using fnc_ptr = uint8_t (read_bits::*)(uint16_t &);

	static constexpr auto ptrs = std::array<fnc_ptr, 4>{
		&read_bits::read_part<true, 6, 2>,
		&read_bits::read_part<true, 4, 4>,
		&read_bits::read_part<true, 2, 6>,
		&read_bits::read_part<false, 0, 0>,
	};

	uint8_t read() {
		// read | previous shift | new shift
		// 1    | <<6            | >>2
		// 1    | <<4            | >>4
		// 1    | <<2            | >>6
		// 0    | <<0            | >>0

		return (this->*ptrs[(counter++) % 4u])(this->tmp);
	}

	char read_encoded() {
		return encoding_properties<encoding::base64>::alphabet[read()];
	}

	auto read_block() -> std::array<uint8_t, 4> {
		uint16_t previous = 0;
		return std::array<uint8_t, 4>{
			read_part<true, 6, 2>(previous),
			read_part<true, 4, 4>(previous),
			read_part<true, 2, 6>(previous),
			read_part<false, 0, 0>(previous)};
	}

	/*
	shift values:
		0: ((tmp = read()) >> 2) [AAAAaa]
		1: (((tmp << 4) ((tmp = read()) >> 4)) & mask) [aaBBBB]
		2: (((tmp << 2) ((tmp = read()) >> 6)) & mask) [bbbbCC]
		3: (tmp & mask) [CCcccc]
	*/
};

template <std::ranges::forward_range Range, typename Encoding = hana::encoding::base64>
requires byte_range<Range>
class encode_view {
private:
	using property = encoding_properties<Encoding>;

	struct iterator {
		constexpr iterator() noexcept: it{}, end{} { }
		constexpr iterator(std::ranges::iterator_t<Range> _it, std::ranges::sentinel_t<Range> _end) noexcept: it{_it}, end{_end} { }
		iterator(const iterator &) = default;
		iterator(iterator &&) noexcept = default;

		iterator & operator=(const iterator &) = default;
		iterator & operator=(iterator &&) noexcept = default;

		~iterator() noexcept = default;

		std::ranges::iterator_t<Range> it;
		std::ranges::sentinel_t<Range> end;

		using buffer_type = std::array<char, property::input_block_size>;

		buffer_type buffer;

		constexpr buffer_type read_buffer() {
			return property::read_buffer(it, end);
		}

		using value_type = char;
		using difference_type = ptrdiff_t;

		constexpr iterator & operator++() noexcept {
			++it; // this should be conditional only when we really need to read
			return *this;
		}

		constexpr iterator operator++(int) noexcept {
			auto copy = *this;
			this->operator++();
			return copy;
		}

		constexpr value_type operator*() const noexcept {
			return property::alphabet[static_cast<uint8_t>(*it)];
		}

		constexpr friend bool operator==(const iterator & lhs, const iterator & rhs) noexcept {
			return lhs.it == rhs.it;
		}
	};

	struct sentinel {
		constexpr friend bool operator==(sentinel, const iterator & it) noexcept {
			return it.it == it.end;
		}
	};

	Range original;

public:
	constexpr encode_view(Range in) noexcept: original{std::move(in)} { }

	encode_view(const encode_view &) = default;
	encode_view(encode_view &&) noexcept = default;

	encode_view & operator=(const encode_view &) = default;
	encode_view & operator=(encode_view &&) noexcept = default;

	~encode_view() noexcept = default;

	constexpr auto begin() const {
		return iterator{original.begin(), original.end()};
	}

	constexpr auto end() const {
		return sentinel{};
	}

	constexpr size_t size() const requires std::ranges::sized_range<Range> {
		return property::calculate_size(std::size(original));
	}

	template <typename CharT = char> constexpr friend auto to_string(const encode_view & view) {
		auto output = std::basic_string<CharT>();
		output.resize(view.size());
		auto it = output.begin();

		for (char c: view) { // missing std::ranges::copy?
			*it++ = c;
		}

		// assert(it == output.end());

		return output;
	}
};

template <typename Encoding = hana::encoding::base64> struct encode_action {
	template <std::ranges::forward_range Range> requires byte_range<Range>
	constexpr friend auto operator|(Range && range, encode_action) noexcept {
		return encode_view<Range, Encoding>(std::forward<Range>(range));
	}

	template <std::ranges::forward_range Range> requires byte_range<Range>
	constexpr auto operator()(Range && range) const noexcept {
		return encode_view<Range, Encoding>(std::forward<Range>(range));
	}
};

template <typename Encoding = hana::encoding::base64> constexpr auto encode = encode_action<Encoding>{};

constexpr auto base64_encode = encode_action<hana::encoding::base64>{};
constexpr auto base32_encode = encode_action<hana::encoding::base32>{};

constexpr auto base16_encode = encode_action<hana::encoding::base16>{};
constexpr auto hexdec_encode = encode_action<hana::encoding::base16>{};

constexpr auto base8_encode = encode_action<hana::encoding::base8>{};
constexpr auto base4_encode = encode_action<hana::encoding::base4>{};
constexpr auto base2_encode = encode_action<hana::encoding::base2>{};

} // namespace hana

#endif

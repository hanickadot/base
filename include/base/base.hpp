#ifndef BASE_BASE_HPP
#define BASE_BASE_HPP

#include <bit>
#include <numeric>
#include <ranges>
#include <string>
#include <cassert>
#include <concepts>
#include <cstddef>

namespace hana {

namespace encoding {

	struct base64 {
		static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		static constexpr char padding = '=';
	};

	struct base32 {
		static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
		static constexpr char padding = '=';
	};

	struct base16 {
		static constexpr char alphabet[] = "0123456789abcdef";
	};

	struct base8 {
		static constexpr char alphabet[] = "01234567";
	};

	struct base4 {
		static constexpr char alphabet[] = "0123";
	};

	struct base2 {
		static constexpr char alphabet[] = "01";
	};

} // namespace encoding

template <typename T> concept byte_like = std::same_as<T, std::byte> || std::same_as<T, char> || std::same_as<T, signed char> || std::same_as<T, unsigned char> || std::same_as<T, uint8_t> || std::same_as<T, int8_t>;

template <typename Encoding> struct encoding_properties {
	static_assert(Encoding::alphabet[std::size(Encoding::alphabet) - 1u] == '\0'); // must be zero-terminated

	static constexpr size_t size = std::size(Encoding::alphabet) - 1u;

	static_assert(std::popcount(size) == 1u); // must be 2^n

	static constexpr size_t bits = std::countr_zero(size);

	static_assert(bits <= 8);
	static_assert(bits > 0);

	static constexpr bool has_padding = requires() { {Encoding::padding} -> byte_like; };

	static constexpr size_t block_size_bits = std::lcm(bits, 8);
	static constexpr size_t input_block_size = block_size_bits / 8;
	static constexpr size_t output_block_size = block_size_bits / bits;

	static consteval auto calculate_alphabet() {
		std::array<char, 256> output;

		for (size_t i = 0; i != 256; ++i) {
			const uint8_t encoded_bits = static_cast<uint8_t>(i) >> (8u - bits);
			output[i] = Encoding::alphabet[encoded_bits];
		}

		return output;
	}

	static constexpr auto alphabet = calculate_alphabet();

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

template <typename T> concept byte_like_range = requires() {
	requires std::ranges::range<T>;
	requires byte_like<std::ranges::range_value_t<T>>;
};

// [AAAA aaaa] [BBBB bbbb] [CCCC cccc] [DDDD dddd] [EEEE eeee] (base256)
// [AAAAaa] [aaBBBB] [bbbbCC] [CCcccc] (base64)
// [AAAAa] [aaaBB] [BBbbb] [bCCCC] [ccccD] [DDDdd] [ddEEE] [Eeeee] (base32)
// [AAAA] [aaaa] (base16)
// [AAA] [Aaa] [aaB] [BBB] [bbb] [bCC] [CCc] [ccc] (base8)
// [AA] [AA] [aa] [aa] (base4)

template <std::ranges::forward_range Range, typename Encoding = hana::encoding::base64>
requires byte_like_range<Range>
class encode_view {
private:
	using property = encoding_properties<Encoding>;

	struct iterator {
		std::ranges::iterator_t<Range> it;
		std::ranges::sentinel_t<Range> end;

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
	template <std::ranges::forward_range Range> requires byte_like_range<Range>
	constexpr friend auto operator|(Range && range, encode_action) noexcept {
		return encode_view<Range, Encoding>(std::forward<Range>(range));
	}

	template <std::ranges::forward_range Range> requires byte_like_range<Range>
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

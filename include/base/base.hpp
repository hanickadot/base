#ifndef HANA_BASE_HPP
#define HANA_BASE_HPP

#include "chunk-of-bits.hpp"
#include "encodings.hpp"
#include <bit>

namespace hana {

template <typename Encoding> struct encoding_properties {
	static constexpr size_t size = std::size(Encoding::alphabet) - 1u;
	static_assert(std::popcount(size) == 1u, "Size of encoding's alphabet must be power-of-two");

	static constexpr size_t bits = std::countr_zero(size);

	static constexpr char padding = [] {
		if constexpr (padded_encoding<Encoding>) {
			return Encoding::padding;
		} else {
			return '\0';
		}
	}();
};

template <typename Encoding, typename CharT, typename R> struct encode_to_view {
	using properties = encoding_properties<Encoding>;
	using chunk_view = hana::chunk_of_bits_view<properties::bits, true, R>;

	struct sentinel {
		[[no_unique_address]] chunk_view::sentinel end;
	};

	struct iterator {
		using difference_type = intptr_t;
		using value_type = CharT;

		chunk_view::iterator it;

		constexpr iterator & operator++() noexcept {
			++it;
			return *this;
		}
		constexpr iterator operator++(int) noexcept {
			auto copy = *this;
			++it;
			return copy;
		}

		constexpr value_type operator*() const noexcept {
			const auto tmp = *it;
			if constexpr (!chunk_view::aligned) {
				// TODO: do without condition
				if (tmp.is_padding()) {
					return properties::padding;
				}
			}
			return static_cast<value_type>(Encoding::alphabet[static_cast<unsigned>(tmp.value)]);
		}

		constexpr friend bool operator==(const iterator &, const iterator &) noexcept = default;

		constexpr friend bool operator==(const iterator & lhs, const sentinel & rhs) noexcept {
			return lhs.it == rhs.end;
		}
	};

	chunk_view input;

	constexpr encode_to_view(R _input): input{_input} { }

	constexpr auto begin() const noexcept {
		return iterator{input.begin()};
	}

	constexpr auto end() const noexcept {
		return sentinel{input.end()};
	}

	constexpr size_t size() const noexcept requires(std::ranges::sized_range<R>) {
		return input.size();
	}
};

template <typename Encoding, typename CharT = char> struct encode_to_action {
	template <std::ranges::input_range R> constexpr friend auto operator|(R && input, encode_to_action action) {
		return action.operator()<R>(std::forward<R>(input));
	}
	template <std::ranges::input_range R> constexpr auto operator()(R && input) {
		return encode_to_view<Encoding, CharT, R>(std::forward<R>(input));
	}
};

template <typename Encoding, typename CharT = char> constexpr auto encode_to = encode_to_action<Encoding, CharT>{};

constexpr auto binary_encode = encode_to<encoding::base2, char>;
constexpr auto base2_encode = encode_to<encoding::base2, char>;
constexpr auto base4_encode = encode_to<encoding::base4, char>;
constexpr auto base8_encode = encode_to<encoding::base8, char>;
constexpr auto hexdec_encode = encode_to<encoding::base16, char>;
constexpr auto base16_encode = encode_to<encoding::base16, char>;
constexpr auto base32_encode = encode_to<encoding::base32, char>;
constexpr auto base64_encode = encode_to<encoding::base64, char>;
} // namespace hana

#endif

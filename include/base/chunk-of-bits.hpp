#ifndef HANA_CHUNK_OF_BITS_HPP
#define HANA_CHUNK_OF_BITS_HPP

#include "bit-buffer.hpp"
#include <ranges>

namespace hana {

template <size_t Bits, bool AllowPadding, std::ranges::input_range Input> requires std::integral<std::ranges::range_value_t<Input>> struct chunk_of_bits_view {
	using input_value_type = std::ranges::range_value_t<Input>;

	static constexpr size_t output_value_bit_size = Bits;
	static constexpr size_t input_value_bit_size = sizeof(input_value_type) * 8u;
	using buffer_t = hana::bit_buffer<output_value_bit_size, input_value_bit_size>;
	using buffer_size_t = buffer_t::size_type;
	Input input;

	struct result_type {
		buffer_t::out_type value;
		buffer_size_t missing_bits;

		constexpr bool is_padding() const noexcept {
			return missing_bits == output_value_bit_size;
		}
	};

	struct sentinel { };

	struct iterator {
		using value_type = result_type;
		using difference_type = intptr_t;

		std::ranges::iterator_t<Input> it;
		[[no_unique_address]] std::ranges::sentinel_t<Input> end;

		buffer_t buffer{};
		buffer_size_t missing_bits{0};

		constexpr iterator(std::ranges::iterator_t<Input> _it, std::ranges::sentinel_t<Input> _end) noexcept: it{std::move(_it)}, end{std::move(_end)} {
			// initialize
			feed_buffer();
		}

		iterator(const iterator &) = default;
		iterator(iterator &&) = default;

		iterator & operator=(const iterator &) = default;
		iterator & operator=(iterator &&) = default;

		constexpr void feed_buffer() noexcept {
			while (!buffer.has_bits_for_pop()) {
				if (it == end) {
					if constexpr (AllowPadding) {
						missing_bits = buffer.push_zeros_to_align();
					} else {
						missing_bits = buffer.push_zeros_to_align();
					}

					break;
				}
				buffer.push(static_cast<std::make_unsigned_t<input_value_type>>(*it));
				++it;
			}
		}

		constexpr iterator & operator++() noexcept {
			buffer.pop();
			feed_buffer();
			return *this;
		}

		constexpr iterator operator++(int) noexcept {
			auto copy = *this;
			this->operator++();
			return copy;
		}

		constexpr auto operator*() const noexcept {
			// assert(output_value_bit_size >= missing_bits);
			return value_type{buffer.front(), missing_bits};
		}

		constexpr friend bool operator==(const iterator & lhs, const iterator & rhs) noexcept {
			return lhs.it == rhs.it;
		}

		constexpr friend bool operator==(const iterator & self, sentinel) noexcept {
			return self.buffer.empty();
		}
	};

	constexpr chunk_of_bits_view(Input _input) noexcept: input{_input} { }

	constexpr auto begin() const noexcept {
		return iterator{input.begin(), input.end()};
	}

	constexpr auto end() const noexcept {
		return sentinel{};
	}
};

template <size_t Bits, bool AllowPadding> struct chunk_of_bits_action {
	template <std::ranges::input_range R> constexpr friend auto operator|(R && input, chunk_of_bits_action action) {
		return action.operator()<R>(std::forward<R>(input));
	}
	template <std::ranges::input_range R> constexpr auto operator()(R && input) {
		return chunk_of_bits_view<Bits, AllowPadding, R>(std::forward<R>(input));
	}
};

template <size_t Bits, bool AllowPadding = false> constexpr auto chunk_of_bits = chunk_of_bits_action<Bits, AllowPadding>{};

} // namespace hana

#endif

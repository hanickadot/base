#ifndef BASE_BASE_HPP
#define BASE_BASE_HPP

#include <ranges>
#include <concepts>
#include <cstddef>

namespace hana {

namespace encoding {

	struct base64 {
		static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		static constexpr char padding = '=';
	};

} // namespace encoding

template <typename T> concept byte_like = std::same_as<T, std::byte> || std::same_as<T, char> || std::same_as<T, signed char> || std::same_as<T, unsigned char> || std::same_as<T, uint8_t> || std::same_as<T, int8_t>;

template <typename T> concept byte_like_range = requires() {
	requires std::ranges::range<T>;
	requires byte_like<std::ranges::range_value_t<T>>;
};

template <std::ranges::forward_range Range, typename Encoding = hana::encoding::base64>
requires byte_like_range<Range>
class encode_view {
private:
	using original_iterator_t = std::ranges::iterator_t<Range>;
	using original_sentinel_t = std::ranges::sentinel_t<Range>;

	struct iterator {
		original_iterator_t it;
		original_sentinel_t end;

		using value_type = char;
		using difference_type = ptrdiff_t;

		constexpr iterator & operator++() noexcept {
			++it;
			return *this;
		}

		constexpr iterator operator++(int) noexcept {
			auto copy = *this;
			this->operator++();
			return copy;
		}

		constexpr value_type operator*() const noexcept {
			return static_cast<char>(*it);
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
		return 42;
	}
};

template <typename Encoding = hana::encoding::base64> struct encode_action {
	template <std::ranges::forward_range Range> requires byte_like_range<Range>
	constexpr friend auto operator|(Range && range, encode_action) noexcept {
		return encode_view<Range, Encoding>(std::forward<Range>(range));
	}
};

template <typename Encoding = hana::encoding::base64> constexpr auto encode = encode_action<Encoding>{};

} // namespace hana

#endif

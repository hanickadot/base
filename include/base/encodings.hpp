#ifndef BASE_ENCODINGS_HPP
#define BASE_ENCODINGS_HPP

#include "concepts.hpp"

namespace hana {

namespace encoding {

	struct base64 {
		static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		static constexpr char padding = '=';
	};

	static_assert(padded_encoding<base64>);

	struct base32 {
		static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
		static constexpr char padding = '=';
	};

	struct base16 {
		static constexpr char alphabet[] = "0123456789abcdef";
	};

	struct base8 {
		static constexpr char alphabet[] = "01234567";
		static constexpr char padding = '=';
	};

	struct base4 {
		static constexpr char alphabet[] = "0123";
	};

	struct base2 {
		static constexpr char alphabet[] = "01";
	};

} // namespace encoding

} // namespace hana

#endif

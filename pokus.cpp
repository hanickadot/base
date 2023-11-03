#include <base/base.hpp>
#include <iostream>
#include <string>
#include <string_view>

using namespace std::string_view_literals;

int main() {
	const auto input = std::string{"Many hands make light work."};
	const auto encoded = input | hana::base64_encode;

	auto it = encoded.begin();
	const auto end = encoded.end();

	static_assert(std::ranges::forward_range<decltype(input)>);
	static_assert(std::ranges::forward_range<decltype(encoded)>);

	static_assert(std::ranges::sized_range<decltype(input)>);
	static_assert(std::ranges::sized_range<decltype(encoded)>);

	static_assert(std::forward_iterator<decltype(it)>);
	static_assert(std::semiregular<std::remove_cvref_t<decltype(end)>>);
	static_assert(std::sentinel_for<std::remove_cvref_t<decltype(end)>, decltype(it)>);

	std::cout << "size(input)   = " << input.size() << "\n";
	std::cout << "size(encoded) = " << encoded.size() << "\n";

	constexpr auto expected = "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu"sv;

	std::cout << "input    = '" << input << "' (length = " << input.size() << ")\n";
	std::cout << "expected = '" << expected << "' (length = " << expected.size() << ")\n";
	std::cout << "result   = '";
	for (char c: encoded) {
		std::cout << c;
	}

	std::cout << "'\n";

	std::cout << to_string(encoded) << "\n";

	for (size_t i = 0; i != 64; ++i) {
		using base64 = hana::encoding_properties<hana::encoding::base64>;
		std::cout << i << " -> " << base64::calculate_size(i) << "\n";
	}
}
#include <base/base.hpp>
#include <iostream>
#include <string>

int main() {
	const auto input = std::string{"Many hands make light work."};
	const auto encoded = input | hana::encode<hana::encoding::base64>;

	auto it = encoded.begin();
	const auto end = encoded.end();

	static_assert(std::ranges::forward_range<decltype(input)>);
	static_assert(std::ranges::forward_range<decltype(encoded)>);

	static_assert(std::forward_iterator<decltype(it)>);
	static_assert(std::semiregular<std::remove_cvref_t<decltype(end)>>);
	static_assert(std::sentinel_for<std::remove_cvref_t<decltype(end)>, decltype(it)>);

	for (char c: encoded) {
		std::cout << c;
	}

	std::cout << "\n";
}
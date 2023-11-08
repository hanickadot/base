#include <catch2/catch_test_macros.hpp>
#include <base/chunk-of-bits.hpp>

using namespace std::string_view_literals;

TEST_CASE("chunk(1)") {
	const auto four_bits = "hello there"sv | hana::chunk_of_bits<4>;

	REQUIRE(four_bits.size() == 11 * 2);

	const auto four_bits_of_empty = ""sv | hana::chunk_of_bits<4>;

	REQUIRE(four_bits_of_empty.size() == 0);

	const auto four_bits_of_bytes = std::array<std::byte, 4>{} | hana::chunk_of_bits<4>;

	REQUIRE(four_bits_of_bytes.size() == 8);
}
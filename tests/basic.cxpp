#include <catch2/catch_test_macros.hpp>
#include <base/base.hpp>
#include <string_view>

using namespace std::string_view_literals;

TEST_CASE("zero length input is always zero length output") {
	REQUIRE((""sv | hana::base2_encode).size() == 0);
	REQUIRE((""sv | hana::base4_encode).size() == 0);
	REQUIRE((""sv | hana::base8_encode).size() == 0);
	REQUIRE((""sv | hana::base16_encode).size() == 0);
	REQUIRE((""sv | hana::base32_encode).size() == 0);
	REQUIRE((""sv | hana::base64_encode).size() == 0);
}

TEST_CASE("one character input is always one block size") {
	REQUIRE(("x"sv | hana::base2_encode).size() == 8);
	REQUIRE(("x"sv | hana::base4_encode).size() == 4);
	REQUIRE(("x"sv | hana::base8_encode).size() == 8);
	REQUIRE(("x"sv | hana::base16_encode).size() == 2);
	REQUIRE(("x"sv | hana::base32_encode).size() == 8);
	REQUIRE(("x"sv | hana::base64_encode).size() == 4);
}

TEST_CASE("two character input is always one block size") {
	REQUIRE(("xy"sv | hana::base2_encode).size() == 16);
	REQUIRE(("xy"sv | hana::base4_encode).size() == 8);
	REQUIRE(("xy"sv | hana::base8_encode).size() == 8);
	REQUIRE(("xy"sv | hana::base16_encode).size() == 4);
	REQUIRE(("xy"sv | hana::base32_encode).size() == 8);
	REQUIRE(("xy"sv | hana::base64_encode).size() == 4);
}

TEST_CASE("base64 encode test-string") {
	// const auto result = to_string("Many hands make light work."sv | hana::base64_encode);
	//  REQUIRE(result == "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu");
}

TEST_CASE("reading bytes") {
	const auto input = std::array<uint8_t, 3>{0xFF, 0x00, 0xEE};
	auto reader = hana::read_bits{input.begin(), input.end()};

	REQUIRE(reader.read_byte() == 0xFFu);
	REQUIRE(reader.read_byte() == 0x00u);
	REQUIRE(reader.read_byte() == 0xEEu);

	REQUIRE(reader.it == reader.end);

	REQUIRE(reader.read_byte() != 0u);
	REQUIRE(reader.read_byte() != 0u);
	REQUIRE(reader.read_byte() != 0u);
	REQUIRE(reader.read_byte() != 0u);
}

TEST_CASE("obtaining bits") {
	const auto input = std::array<uint8_t, 3>{0xFF, 0x00, 0xEE};
	auto reader = hana::read_bits{input.begin(), input.end()};

	REQUIRE(reader.read() == 0b111111u);
	REQUIRE(reader.read() == 0b110000u);
	REQUIRE(reader.read() == 0b000011u);
	REQUIRE(reader.read() == 0b101110u);

	REQUIRE(reader.it == reader.end);
}

TEST_CASE("obtaining bits (with padding 1)") {
	const auto input = std::array<uint8_t, 2>{0xFF, 0x00};
	auto reader = hana::read_bits{input.begin(), input.end()};

	REQUIRE(reader.read() == 0b111111u);
	REQUIRE(reader.read() == 0b110000u);
	REQUIRE(reader.read() == 0b000000u);
	REQUIRE(reader.read() >= 0b11000000u);

	REQUIRE(reader.it == reader.end);
}

TEST_CASE("obtaining bits (with padding 2)") {
	const auto input = std::array<uint8_t, 1>{0xFF};
	auto reader = hana::read_bits{input.begin(), input.end()};

	REQUIRE(reader.read() == 0b111111u);
	REQUIRE(reader.read() == 0b110000u);
	REQUIRE(reader.read() >= 0b11000000u);
	REQUIRE(reader.read() >= 0b11000000u);

	REQUIRE(reader.it == reader.end);
}

TEST_CASE("obtaining encoded bits") {
	const auto input = std::array<uint8_t, 3>{0xFF, 0x00, 0xEE};
	auto reader = hana::read_bits{input.begin(), input.end()};

	REQUIRE(reader.read_encoded() == '/');
	REQUIRE(reader.read_encoded() == 'w');
	REQUIRE(reader.read_encoded() == 'D');
	REQUIRE(reader.read_encoded() == 'u');

	REQUIRE(reader.it == reader.end);
}

static_assert(hana::encoding_properties<hana::encoding::base64>::padding == '=');

TEST_CASE("obtaining encoded bits (with padding 1)") {
	const auto input = std::array<uint8_t, 2>{0xFF, 0x00};
	auto reader = hana::read_bits{input.begin(), input.end()};

	REQUIRE(reader.read_encoded() == '/');
	REQUIRE(reader.read_encoded() == 'w');
	REQUIRE(reader.read_encoded() == 'A');
	REQUIRE(reader.read_encoded() == '=');

	REQUIRE(reader.it == reader.end);
}

TEST_CASE("obtaining encoded bits (with padding 2)") {
	const auto input = std::array<uint8_t, 1>{0xFF};
	auto reader = hana::read_bits{input.begin(), input.end()};

	REQUIRE(reader.read_encoded() == '/');
	REQUIRE(reader.read_encoded() == 'w');
	REQUIRE(reader.read_encoded() == '=');
	REQUIRE(reader.read_encoded() == '=');

	REQUIRE(reader.it == reader.end);
}

TEST_CASE("obtaining encoded bits (large)") {
	const auto input = std::array<uint8_t, 5>{0xFF, 0x00, 0xEE, 0xFF, 0x00};
	auto reader = hana::read_bits{input.begin(), input.end()};

	REQUIRE(reader.read_encoded() == '/');
	REQUIRE(reader.read_encoded() == 'w');
	REQUIRE(reader.read_encoded() == 'D');
	REQUIRE(reader.read_encoded() == 'u');
	REQUIRE(reader.read_encoded() == '/');
	REQUIRE(reader.read_encoded() == 'w');
	REQUIRE(reader.read_encoded() == 'A');
	REQUIRE(reader.read_encoded() == '=');

	REQUIRE(reader.it == reader.end);
}

TEST_CASE("obtaining bit blocks") {
	const auto input = std::array<uint8_t, 3>{0xFF, 0x00, 0xEE};
	auto reader = hana::read_bits{input.begin(), input.end()};

	const auto block = reader.read_block();
	REQUIRE(block[0] == 0b111111u);
	REQUIRE(block[1] == 0b110000u);
	REQUIRE(block[2] == 0b000011u);
	REQUIRE(block[3] == 0b101110u);

	REQUIRE(reader.it == reader.end);
}
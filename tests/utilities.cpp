#include <catch2/catch_test_macros.hpp>
#include <base/utilities.hpp>

using namespace std::string_view_literals;

TEST_CASE("masks") {
	REQUIRE((hana::mask<0>) == 0b00000000u);
	REQUIRE((hana::mask<1>) == 0b00000001u);
	REQUIRE((hana::mask<2>) == 0b00000011u);
	REQUIRE((hana::mask<3>) == 0b00000111u);
	REQUIRE((hana::mask<4>) == 0b00001111u);
	REQUIRE((hana::mask<5>) == 0b00011111u);
	REQUIRE((hana::mask<6>) == 0b00111111u);
	REQUIRE((hana::mask<7>) == 0b01111111u);
	REQUIRE((hana::mask<8>) == 0b11111111u);

	REQUIRE((hana::mask<9>) == 0b0000000111111111u);
	REQUIRE((hana::mask<10>) == 0b0000001111111111u);
	REQUIRE((hana::mask<11>) == 0b0000011111111111u);
	REQUIRE((hana::mask<12>) == 0b0000111111111111u);
	REQUIRE((hana::mask<13>) == 0b0001111111111111u);
	REQUIRE((hana::mask<14>) == 0b0011111111111111u);
	REQUIRE((hana::mask<15>) == 0b0111111111111111u);
	REQUIRE((hana::mask<16>) == 0b1111111111111111u);
}

TEST_CASE("positive shift") {
	REQUIRE(0b00000001u == (hana::shift<0>(0b1u)));
	REQUIRE(0b00000010u == (hana::shift<1>(0b1u)));
	REQUIRE(0b00000100u == (hana::shift<2>(0b1u)));
	REQUIRE(0b00001000u == (hana::shift<3>(0b1u)));
	REQUIRE(0b00010000u == (hana::shift<4>(0b1u)));
	REQUIRE(0b00100000u == (hana::shift<5>(0b1u)));
	REQUIRE(0b01000000u == (hana::shift<6>(0b1u)));
	REQUIRE(0b10000000u == (hana::shift<7>(0b1u)));
}

TEST_CASE("negative shift") {
	REQUIRE(0b10000000u == (hana::shift<(-0)>(0b10000000u)));
	REQUIRE(0b01000000u == (hana::shift<(-1)>(0b10000000u)));
	REQUIRE(0b00100000u == (hana::shift<(-2)>(0b10000000u)));
	REQUIRE(0b00010000u == (hana::shift<(-3)>(0b10000000u)));
	REQUIRE(0b00001000u == (hana::shift<(-4)>(0b10000000u)));
	REQUIRE(0b00000100u == (hana::shift<(-5)>(0b10000000u)));
	REQUIRE(0b00000010u == (hana::shift<(-6)>(0b10000000u)));
	REQUIRE(0b00000001u == (hana::shift<(-7)>(0b10000000u)));
}

TEST_CASE("extract hexdec") {
	auto read_byte = [value = 0xABCDEFu]() mutable -> uint8_t {
		const uint8_t r = static_cast<uint8_t>(value & 0xFFu);
		value >>= 8;
		return r;
	};

	auto ext = hana::extractor<4, uint16_t>{};

	auto get_lo = [&]() { return ext.extract<nullptr, 0>(read_byte); };
	auto get_hi = [&]() { return ext.extract<(-4), nullptr>(read_byte); };

	REQUIRE(get_lo() == 0xFu);
	REQUIRE(get_hi() == 0xEu);
	REQUIRE(get_lo() == 0xDu);
	REQUIRE(get_hi() == 0xCu);
	REQUIRE(get_lo() == 0xBu);
	REQUIRE(get_hi() == 0xAu);
	REQUIRE(get_lo() == 0);
	REQUIRE(get_hi() == 0);
}

TEST_CASE("extract base64") {
	std::array<uint8_t, 3> buffer{0b101010'11u, 0b1100'1101u, 0b11'101111u};

	auto read_byte = [it = buffer.begin()]() mutable -> uint8_t {
		return *it++;
	};

	auto ext = hana::extractor<6, uint16_t>{};

	/*

// [AAAA aaaa] [BBBB bbbb] [CCCC cccc] [DDDD dddd] [EEEE eeee] (base256 = byte)

// [AAAAaa] [aaBBBB] [bbbbCC] [CCcccc] (base64)

		&read_bits::read_part<true, 6, 2, 0b00'111111u>,
		&read_bits::read_part<true, 4, 4, 0b00'111111u>,
		&read_bits::read_part<true, 2, 6, 0b00'111111u>,
		&read_bits::read_part<false, 0, 0, 0b00'111111u>,
	*/

	[[maybe_unused]] auto get_a = [&]() { return ext.extract<nullptr, (-2)>(read_byte); };
	[[maybe_unused]] auto get_b = [&]() { return ext.extract<(4), (-4)>(read_byte); };
	[[maybe_unused]] auto get_c = [&]() { return ext.extract<(2), (-6)>(read_byte); };
	[[maybe_unused]] auto get_d = [&]() { return ext.extract<(0), nullptr>(read_byte); };

	REQUIRE(get_a() == 0b101010u);
	REQUIRE(get_b() == 0b111100u);
	REQUIRE(get_c() == 0b110111u);
	REQUIRE(get_d() == 0b101111u);
}

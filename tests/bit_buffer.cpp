#include <catch2/catch_test_macros.hpp>
#include <base/chunk-of-bits.hpp>

using namespace std::string_view_literals;

TEST_CASE("bit_buffer(1)") {
	auto buffer = hana::bit_buffer<6, 8>{};
	REQUIRE(buffer.empty());
	REQUIRE(buffer.has_capacity_for_push());
	buffer.push(0xFFu);
	REQUIRE(!buffer.empty());
	REQUIRE(buffer.has_bits_for_pop());

	const auto out1 = buffer.front();
	buffer.pop();
	REQUIRE(out1 == 0b111111u);
}

TEST_CASE("bit_buffer(2)") {
	auto buffer = hana::bit_buffer<6, 8>{};
	REQUIRE(buffer.empty());
	REQUIRE(buffer.has_capacity_for_push());

	buffer.push(0xFFu);

	REQUIRE(buffer.has_bits_for_pop());
	REQUIRE_FALSE(buffer.empty());
	REQUIRE(buffer.has_capacity_for_push());
	buffer.push(0xFFu);

	REQUIRE(buffer.has_bits_for_pop());
	REQUIRE_FALSE(buffer.empty());
	REQUIRE(buffer.has_capacity_for_push());
	buffer.push(0xFFu);

	REQUIRE(buffer.has_bits_for_pop());
	REQUIRE_FALSE(buffer.empty());
	REQUIRE_FALSE(buffer.has_capacity_for_push());

	const auto out1 = buffer.front();
	buffer.pop();
	REQUIRE(out1 == 0b111111u);

	const auto out2 = buffer.front();
	buffer.pop();
	REQUIRE(out2 == 0b111111u);

	const auto out3 = buffer.front();
	buffer.pop();
	REQUIRE(out3 == 0b111111u);

	const auto out4 = buffer.front();
	buffer.pop();
	REQUIRE(out4 == 0b111111u);

	REQUIRE(buffer.empty());
	REQUIRE(buffer.has_capacity_for_push());
	REQUIRE_FALSE(buffer.has_bits_for_pop());
}

TEST_CASE("bit_buffer(3) check patterns") {
	auto buffer = hana::bit_buffer<6, 8>{};
	REQUIRE(buffer.empty());
	REQUIRE(buffer.has_capacity_for_push());
	REQUIRE_FALSE(buffer.full());

	buffer.push(0x86u);
	REQUIRE(buffer.has_capacity_for_push());

	buffer.push(0x18u);
	REQUIRE(buffer.has_capacity_for_push());

	buffer.push(0x61u);

	REQUIRE_FALSE(buffer.empty());
	REQUIRE_FALSE(buffer.has_capacity_for_push());
	REQUIRE(buffer.full());

	auto front_and_pop = [&] {
		REQUIRE_FALSE(buffer.empty());
		const auto out1 = buffer.front();
		buffer.pop();
		return out1;
	};

	REQUIRE(front_and_pop() == 0b100001u);
	REQUIRE(front_and_pop() == 0b100001u);
	REQUIRE(front_and_pop() == 0b100001u);
	REQUIRE(front_and_pop() == 0b100001u);

	REQUIRE(buffer.empty());
}
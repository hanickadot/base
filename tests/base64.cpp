#include <catch2/catch_test_macros.hpp>
#include <base/base.hpp>
#include <sstream>

using namespace std::string_view_literals;

static auto materialize(auto && range) -> std::string {
	std::string output;
	for (char c: range) {
		output += c;
	}
	return output;
}

TEST_CASE("base64 basics") {
	const auto view1 = "Man"sv | hana::base64_encode;
	REQUIRE(materialize(view1) == "TWFu");

	const auto view2 = "Ma"sv | hana::base64_encode;
	REQUIRE(materialize(view2) == "TWE=");

	const auto view3 = "M"sv | hana::base64_encode;
	REQUIRE(materialize(view3) == "TQ==");

	const auto empty = ""sv | hana::base64_encode;
	REQUIRE(materialize(empty) == "");
}

TEST_CASE("base32 basics") {
	const auto view1 = "abcde"sv | hana::base32_encode;
	REQUIRE(materialize(view1) == "MFRGGZDF");

	const auto view2 = "abcd"sv | hana::base32_encode;
	REQUIRE(materialize(view2) == "MFRGGZA=");

	const auto view3 = "abc"sv | hana::base32_encode;
	REQUIRE(materialize(view3) == "MFRGG===");

	const auto view4 = "ab"sv | hana::base32_encode;
	REQUIRE(materialize(view4) == "MFRA====");

	const auto view5 = "a"sv | hana::base32_encode;
	REQUIRE(materialize(view5) == "ME======");

	const auto empty = ""sv | hana::base32_encode;
	REQUIRE(materialize(empty) == "");
}

TEST_CASE("hexdec basics") {
	const auto view1 = "Aloha"sv | hana::hexdec_encode;
	REQUIRE(materialize(view1) ==
		"41"
		"6c"
		"6f"
		"68"
		"61");

	const auto empty = ""sv | hana::hexdec_encode;
	REQUIRE(materialize(empty) == "");
}

TEST_CASE("binary basics") {
	const auto view1 = "Aloha"sv | hana::binary_encode;
	REQUIRE(materialize(view1) ==
		"01000001"
		"01101100"
		"01101111"
		"01101000"
		"01100001");

	const auto empty = ""sv | hana::binary_encode;
	REQUIRE(materialize(empty) == "");
}

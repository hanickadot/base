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
	const auto result = to_string("Many hands make light work."sv | hana::base64_encode);
	REQUIRE(result == "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu");
}

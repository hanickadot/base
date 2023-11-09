#include <catch2/catch_test_macros.hpp>
#include <base/base.hpp>
#include <sstream>

using namespace std::string_view_literals;

static auto materialize(const auto & range) {
	using char_type = std::ranges::range_value_t<decltype(range)>;
	std::basic_string<char_type> output;
	for (char_type c: range) {
		output += c;
	}

	std::basic_string<char_type> output2;
	output2.resize(range.size());
	auto it = output2.begin();
	const auto end = output2.end();
	const auto [in, out] = std::ranges::copy(range.begin(), range.end(), it);

	REQUIRE(out == end);
	REQUIRE(in == range.end());
	REQUIRE(output.size() == output2.size());

	return output2;
}

static auto result_size(const auto & range) {
	return range.size();
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

template <typename T> constexpr auto make_array(std::convertible_to<T> auto... values) {
	return std::array<T, sizeof...(values)>{static_cast<T>(values)...};
}

TEST_CASE("base64 value corner-cases") {
	const auto arr = make_array<unsigned char>(0, 0xFFu, 0, 0xFF, 0, 0xFF);
	const auto view1 = arr | hana::base64_encode;
	static_assert(std::input_iterator<decltype(view1.begin())>);
	static_assert(std::ranges::input_range<decltype(view1)>);
	REQUIRE(materialize(view1) == "AP8A/wD/");
}

template <typename...> struct identify;

TEST_CASE("base64 value corner-cases (construct from temporary)") {
	const auto view1 = make_array<unsigned char>(0, 0xFFu, 0, 0xFF, 0, 0xFF) | hana::base64_encode;
	static_assert(std::input_iterator<decltype(view1.begin())>);
	static_assert(std::ranges::input_range<decltype(view1)>);
	REQUIRE(materialize(view1) == "AP8A/wD/");
}

TEST_CASE("base64url basics") {
	const auto view1 = "Man"sv | hana::base64url_encode;
	REQUIRE(materialize(view1) == "TWFu");

	const auto view2 = "Ma"sv | hana::base64url_encode;
	REQUIRE(materialize(view2) == "TWE");

	const auto view3 = "M"sv | hana::base64url_encode;
	REQUIRE(materialize(view3) == "TQ");

	const auto view4 = "ab~"sv | hana::base64url_encode;
	REQUIRE(materialize(view4) == "YWJ-");

	const auto empty = ""sv | hana::base64url_encode;
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

TEST_CASE("z-base32 basics") {
	const auto view1 = "abcde"sv | hana::z_base32_encode;
	REQUIRE(materialize(view1) == "cftgg3df");

	const auto view2 = "abcd"sv | hana::z_base32_encode;
	REQUIRE(materialize(view2) == "cftgg3y");

	const auto view3 = "abc"sv | hana::z_base32_encode;
	REQUIRE(materialize(view3) == "cftgg");

	const auto view4 = "ab"sv | hana::z_base32_encode;
	REQUIRE(materialize(view4) == "cfty");

	const auto view5 = "a"sv | hana::z_base32_encode;
	REQUIRE(materialize(view5) == "cr");

	const auto empty = ""sv | hana::z_base32_encode;
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

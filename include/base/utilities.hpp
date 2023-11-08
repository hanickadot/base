#ifndef BASE_UTILITIES_HPP
#define BASE_UTILITIES_HPP

#include <concepts>

namespace hana {

template <unsigned Bits, std::unsigned_integral T = unsigned> constexpr auto mask = (T{0b1u} << Bits) - 1u;

template <int Shift> [[gnu::always_inline]] constexpr auto shift(std::unsigned_integral auto value) noexcept requires(Shift < 0) {
	using T = std::remove_cvref_t<decltype(value)>;
	static_assert((-Shift) < (sizeof(T) * 8u));
	return static_cast<T>(value >> static_cast<size_t>(-Shift));
}

template <int Shift> [[gnu::always_inline]] constexpr auto shift(std::unsigned_integral auto value) noexcept requires(Shift > 0) {
	using T = std::remove_cvref_t<decltype(value)>;
	static_assert((Shift) < (sizeof(T) * 8u));
	return static_cast<T>(value << static_cast<size_t>(Shift));
}

template <int Shift> [[gnu::always_inline]] constexpr auto shift(std::unsigned_integral auto value) noexcept requires(Shift == 0) {
	return value;
}

template <unsigned Mask, typename Buffer> struct extractor {
	static constexpr auto mask_bits = mask<Mask, Buffer>;
	Buffer buffer{0};

	template <int PreviousShift, int NextShift> constexpr auto extract(auto && cb) noexcept(noexcept(cb())) {
		const auto previous = buffer;
		buffer = cb();

		return (shift<PreviousShift>(previous) | shift<NextShift>(buffer)) & mask_bits;
	}

	template <nullptr_t, int NextShift> constexpr auto extract(auto && cb) noexcept(noexcept(cb())) {
		buffer = cb();
		return shift<NextShift>(buffer) & mask_bits;
	}

	template <int PreviousShift, nullptr_t> constexpr auto extract(auto &&) noexcept {
		return shift<PreviousShift>(buffer) & mask_bits;
	}
};

struct base64_extractor: extractor<5u, uint16_t>  {
	using super = extractor<5u, uint16_t>;
	using super::extract;
	
	/*
	[[maybe_unused]] auto get_a = [&]() { return ext.extract<nullptr, (-2)>(read_byte); };
	[[maybe_unused]] auto get_b = [&]() { return ext.extract<(4), (-4)>(read_byte); };
	[[maybe_unused]] auto get_c = [&]() { return ext.extract<(2), (-6)>(read_byte); };
	[[maybe_unused]] auto get_d = [&]() { return ext.extract<(0), nullptr>(read_byte); };
	*/
	
	
};

} // namespace hana

#endif

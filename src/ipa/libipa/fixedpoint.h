/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Paul Elder <paul.elder@ideasonboard.com>
 *
 * Fixed / floating point conversions
 */

#pragma once

#include <cmath>
#include <type_traits>

#include "quantized.h"

namespace libcamera {

namespace ipa {

#ifndef __DOXYGEN__
template<unsigned int I, unsigned int F, typename R, typename T,
	 std::enable_if_t<std::is_integral_v<R> &&
			  std::is_floating_point_v<T>> * = nullptr>
#else
template<unsigned int I, unsigned int F, typename R, typename T>
#endif
constexpr R floatingToFixedPoint(T number)
{
	static_assert(sizeof(int) >= sizeof(R));
	static_assert(I + F <= sizeof(R) * 8);

	/*
	 * The intermediate cast to int is needed on arm platforms to properly
	 * cast negative values. See
	 * https://embeddeduse.com/2013/08/25/casting-a-negative-float-to-an-unsigned-int/
	 */
	R mask = (1 << (F + I)) - 1;
	R frac = static_cast<R>(static_cast<int>(std::round(number * (1 << F)))) & mask;

	return frac;
}

#ifndef __DOXYGEN__
template<unsigned int I, unsigned int F, typename R, typename T,
	 std::enable_if_t<std::is_floating_point_v<R> &&
			  std::is_integral_v<T>> * = nullptr>
#else
template<unsigned int I, unsigned int F, typename R, typename T>
#endif
constexpr R fixedToFloatingPoint(T number)
{
	static_assert(sizeof(int) >= sizeof(T));
	static_assert(I + F <= sizeof(T) * 8);

	if constexpr (std::is_unsigned_v<T>)
		return static_cast<R>(number) / static_cast<R>(T{ 1 } << F);

	/*
	 * Recreate the upper bits in case of a negative number by shifting the sign
	 * bit from the fixed point to the first bit of the unsigned and then right shifting
	 * by the same amount which keeps the sign bit in place.
	 * This can be optimized by the compiler quite well.
	 */
	int remaining_bits = sizeof(int) * 8 - (I + F);
	int t = static_cast<int>(static_cast<unsigned>(number) << remaining_bits) >> remaining_bits;
	return static_cast<R>(t) / static_cast<R>(1 << F);
}

template<unsigned int I, unsigned int F, typename T>
struct FixedPointQTraits {
private:
	static_assert(std::is_integral_v<T>, "FixedPointQTraits: T must be integral");
	using UT = std::make_unsigned_t<T>;

	static constexpr unsigned int bits = I + F;
	static_assert(bits <= sizeof(UT) * 8, "FixedPointQTraits: too many bits for type UT");

	/*
	 * If fixed point storage is required with more than 24 bits, consider
	 * updating this implementation to use double-precision floating point.
	 */
	static_assert(bits <= 24, "Floating point precision may be insufficient for more than 24 bits");

	static constexpr UT bitMask = bits < sizeof(UT) * 8
				    ? (UT{ 1 } << bits) - 1
				    : ~UT{ 0 };

public:
	using QuantizedType = UT;

	static constexpr UT qMin = std::is_signed_v<T>
				 ? -(UT{ 1 } << (bits - 1))
				 : 0;

	static constexpr UT qMax = std::is_signed_v<T>
				 ? (UT{ 1 } << (bits - 1)) - 1
				 : bitMask;

	static constexpr float toFloat(QuantizedType q)
	{
		return fixedToFloatingPoint<I, F, float, T>(q);
	}

	static constexpr float min = fixedToFloatingPoint<I, F, float>(static_cast<T>(qMin));
	static constexpr float max = fixedToFloatingPoint<I, F, float>(static_cast<T>(qMax));

	static_assert(min < max, "FixedPointQTraits: Minimum must be less than maximum");

	/* Conversion functions required by Quantized<Traits> */
	static QuantizedType fromFloat(float v)
	{
		v = std::clamp(v, min, max);
		return floatingToFixedPoint<I, F, T, float>(v);
	}
};

namespace details {

template<unsigned int Bits>
constexpr auto qtype()
{
	static_assert(Bits <= 32,
		      "Unsupported number of bits for quantized type");

	if constexpr (Bits <= 8)
		return int8_t();
	else if constexpr (Bits <= 16)
		return int16_t();
	else if constexpr (Bits <= 32)
		return int32_t();
}

} /* namespace details */

template<unsigned int I, unsigned int F>
using Q = Quantized<FixedPointQTraits<I, F, decltype(details::qtype<I + F>())>>;

template<unsigned int I, unsigned int F>
using UQ = Quantized<FixedPointQTraits<I, F, std::make_unsigned_t<decltype(details::qtype<I + F>())>>>;

} /* namespace ipa */

} /* namespace libcamera */

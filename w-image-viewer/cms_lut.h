#pragma once

#include "pch.h"

//DONT CALL THIS AT RUNTIME!
template <int lut_size>
consteval auto wiv_fill_lut()
{
	std::array<uint16_t, lut_size * lut_size * lut_size * 4> lut{};
	int i{};
	for (int b{}; b < lut_size; ++b)
		for (int g{}; g < lut_size; ++g)
			for (int r{}; r < lut_size; ++r) {
				lut[i++] = r * 65535 / (lut_size - 1);
				lut[i++] = g * 65535 / (lut_size - 1);
				lut[i++] = b * 65535 / (lut_size - 1);
				i++; // Iterate over alpha.
			}
	return lut;
}

// Precompute LUTs since this would be expensive at runtime.
inline constexpr auto WIV_CMS_LUT_33{ wiv_fill_lut<33>() };
inline constexpr auto WIV_CMS_LUT_49{ wiv_fill_lut<49>() };
inline constexpr auto WIV_CMS_LUT_65{ wiv_fill_lut<65>() };
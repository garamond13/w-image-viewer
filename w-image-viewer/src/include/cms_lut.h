#pragma once

#include "pch.h"

//DONT CALL THIS AT RUNTIME!
template <int lut_size>
consteval auto wiv_fill_lut()
{
    std::array<uint16_t, lut_size * lut_size * lut_size * 3> lut = {};
    int i = 0;
    for (int b = 0; b < lut_size; ++b)
        for (int g = 0; g < lut_size; ++g)
            for (int r = 0; r < lut_size; ++r) {
                lut[i++] = r * 65535 / (lut_size - 1);
                lut[i++] = g * 65535 / (lut_size - 1);
                lut[i++] = b * 65535 / (lut_size - 1);
            }
    return lut;
}

// Precompute LUTs since this would be expensive at runtime.
inline constexpr auto WIV_CMS_LUT_33 = wiv_fill_lut<33>();
inline constexpr auto WIV_CMS_LUT_49 = wiv_fill_lut<49>();
inline constexpr auto WIV_CMS_LUT_65 = wiv_fill_lut<65>();
#pragma once

#include "pch.h"

// Source https://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf
template<std::floating_point T>
inline constexpr T ADOBE_RGB_GAMMA{ 2.19921875 };

// AdobeRGB1998
// Needs to be freed with cmsCloseProfile().
cmsHPROFILE cms_create_profile_adobe_rgb() noexcept;

// Needs to be freed with cmsCloseProfile().
cmsHPROFILE cms_create_profile_linear_srgb() noexcept;

// Needs to be freed with cmsCloseProfile().
cmsHPROFILE cms_create_profile_aces_cg() noexcept;

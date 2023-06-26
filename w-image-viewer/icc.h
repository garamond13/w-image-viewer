#pragma once

#include "pch.h"

//AdobeRGB1998
//needs to be freed with cmsCloseProfile()
cmsHPROFILE cms_create_profile_adobe_rgb() noexcept;

//needs to be freed with cmsCloseProfile()
cmsHPROFILE cms_create_profile_linear_srgb() noexcept;

#include "pch.h"
#include "icc.h"

//source https://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf
cmsHPROFILE cms_create_profile_adobe_rgb() noexcept
{
	static constinit const cmsCIExyY whitepoint{ 0.3127, 0.3290, 1.0 };
	static constinit const cmsCIExyYTRIPLE primaries{
		{ 0.6400, 0.3300, 1.0 },
		{ 0.2100, 0.7100, 1.0 },
		{ 0.1500, 0.0600, 1.0 }
	};
	std::array<cmsToneCurve*, 3> tone_curve;
	tone_curve[2] = tone_curve[1] = tone_curve[0] = cmsBuildGamma(nullptr, 2.19921875);
	cmsHPROFILE profile{ cmsCreateRGBProfile(&whitepoint, &primaries, tone_curve.data()) };
	cmsFreeToneCurve(tone_curve[0]);
	return profile;
}

cmsHPROFILE cms_create_profile_linear_srgb() noexcept
{
	static constinit const cmsCIExyY whitepoint{ 0.3127, 0.3290, 1.0 };
	static constinit const cmsCIExyYTRIPLE primaries{
		{ 0.6400, 0.3300, 1.0 },
		{ 0.3000, 0.6000, 1.0 },
		{ 0.1500, 0.0600, 1.0 }
	};
	std::array<cmsToneCurve*, 3> tone_curve;
	tone_curve[2] = tone_curve[1] = tone_curve[0] = cmsBuildGamma(nullptr, 1.0);
	cmsHPROFILE profile{ cmsCreateRGBProfile(&whitepoint, &primaries, tone_curve.data()) };
	cmsFreeToneCurve(tone_curve[0]);
	return profile;
}

//source https://docs.acescentral.com/specifications/acescg/
cmsHPROFILE cms_create_profile_aces_cg() noexcept
{
	static constinit const cmsCIExyY whitepoint{ 0.32168, 0.33767, 1.0 };
	static constinit const cmsCIExyYTRIPLE primaries{
		{ 0.713, 0.293, 1.0 },
		{ 0.165, 0.830, 1.0 },
		{ 0.128, 0.044, 1.0 }
	};
	std::array<cmsToneCurve*, 3> tone_curve;
	tone_curve[2] = tone_curve[1] = tone_curve[0] = cmsBuildGamma(nullptr, 1.0);
	cmsHPROFILE profile{ cmsCreateRGBProfile(&whitepoint, &primaries, tone_curve.data()) };
	cmsFreeToneCurve(tone_curve[0]);
	return profile;
}

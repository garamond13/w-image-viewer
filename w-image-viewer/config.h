#pragma once

#include "pch.h"
#include "range.h"

// Config pairs
//

// WIV_NAME_WINDOW_
inline constexpr auto WIV_NAME_WINDOW_WIDTH_KEY{ "wnd_w" };
#define WIV_NAME_WINDOW_WIDTH_VAL window_width

inline constexpr auto WIV_NAME_WINDOW_HEIGHT_KEY{ "wnd_h" };
#define WIV_NAME_WINDOW_HEIGHT_VAL window_height

inline constexpr auto WIV_NAME_WINDOW_USE_AUTO_DIMS_KEY{ "wnd_autwh" };
#define WIV_NAME_WINDOW_USE_AUTO_DIMS_VAL window_autowh

inline constexpr auto WIV_NAME_WINDOW_NAME_KEY{ "wnd_name" };
#define WIV_NAME_WINDOW_NAME_VAL window_name

// WIV_NAME_CLEAR_
inline constexpr std::array WIV_NAME_CLEAR_COLOR_KEY{ "clr_r", "clr_g", "clr_b" };
#define WIV_NAME_CLEAR_COLOR_VAL clear_color

// WIV_NAME_ALPHA_
inline constexpr std::array WIV_NAME_ALPHA_TILE1_COLOR_KEY{ "a_t1r", "a_t1g", "a_t1b" };
#define WIV_NAME_ALPHA_TILE1_COLOR_VAL alpha_tile1_color

inline constexpr std::array WIV_NAME_ALPHA_TILE2_COLOR_KEY{ "a_t2r", "a_t2g", "a_t2b" };
#define WIV_NAME_ALPHA_TILE2_COLOR_VAL alpha_tile2_color

inline constexpr auto WIV_NAME_ALPHA_TILE_SIZE_KEY{ "a_tsz" };
#define WIV_NAME_ALPHA_TILE_SIZE_VAL alpha_tile_size

// WIV_NAME_KERNEL_
inline constexpr auto WIV_NAME_KERNEL_INDEX_KEY{ "k_i" };
#define WIV_NAME_KERNEL_INDEX_VAL kernel_index

inline constexpr auto WIV_NAME_KERNEL_RADIUS_KEY{ "k_r" };
#define WIV_NAME_KERNEL_RADIUS_VAL kernel_radius

inline constexpr auto WIV_NAME_KERNEL_BLUR_KEY{ "k_b" };
#define WIV_NAME_KERNEL_BLUR_VAL kernel_blur

inline constexpr auto WIV_NAME_KERNEL_PARAMETER1_KEY{ "k_p1" };
#define WIV_NAME_KERNEL_PARAMETER1_VAL kernel_p1

inline constexpr auto WIV_NAME_KERNEL_PARAMETER2_KEY{ "k_p2" };
#define WIV_NAME_KERNEL_PARAMETER2_VAL kernel_p2

inline constexpr auto WIV_NAME_KERNEL_ANTIRINGING_KEY{ "k_ar" };
#define WIV_NAME_KERNEL_ANTIRINGING_VAL kernel_ar

inline constexpr auto WIV_NAME_KERNEL_USE_CYLINDRICAL_KEY{ "k_cyl" };
#define WIV_NAME_KERNEL_USE_CYLINDRICAL_VAL kernel_use_cyl

// WIV_NAME_SIGMOID_
inline constexpr auto WIV_NAME_SIGMOID_USE_KEY{ "sig" };
#define WIV_NAME_SIGMOID_USE_VAL sigmoid_use

inline constexpr auto WIV_NAME_SIGMOID_CONTRAST_KEY{ "sig_c" };
#define WIV_NAME_SIGMOID_CONTRAST_VAL sigmoid_contrast

inline constexpr auto WIV_NAME_SIGMOID_MIDPOINT_KEY{ "sig_m" };
#define WIV_NAME_SIGMOID_MIDPOINT_VAL sigmoid_midpoint


// WIV_NAME_CMS_
inline constexpr auto WIV_NAME_CMS_USE_KEY{ "cms" };
#define WIV_NAME_CMS_USE_VAL cms_use

inline constexpr auto WIV_NAME_CMS_INTENT_KEY{ "cms_itent" };
#define WIV_NAME_CMS_INTENT_VAL cms_intent

inline constexpr auto WIV_NAME_CMS_USE_BLACKPOINT_COMPENSATION_KEY{ "cms_bpc" };
#define WIV_NAME_CMS_USE_BLACKPOINT_COMPENSATION_VAL cms_use_bpc

inline constexpr auto WIV_NAME_CMS_USE_DEFUALT_TO_SRGB_KEY{ "cms_deftosrgb" };
#define WIV_NAME_CMS_USE_DEFUALT_TO_SRGB_VAL cms_use_defualt_to_srgb

inline constexpr auto WIV_NAME_CMS_USE_DEFUALT_TO_ACES_KEY{ "cms_deftoaces" };
#define WIV_NAME_CMS_USE_DEFUALT_TO_ACES_VAL cms_use_default_to_aces

inline constexpr auto WIV_NAME_CMS_PROFILE_DISPLAY_KEY{ "cms_prof" }; // enum WIV_CMS_PROFILE_DISPLAY_
#define WIV_NAME_CMS_PROFILE_DISPLAY_VAL cms_profile_display // enum WIV_CMS_PROFILE_DISPLAY_

inline constexpr auto WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM_KEY{ "cms_prof_cus" };
#define WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM_VAL cms_profile_display_custom

inline constexpr auto WIV_NAME_CMS_LUT_SIZE_KEY{ "cms_lutsz" };
#define WIV_NAME_CMS_LUT_SIZE_VAL cms_lut_size

// WIV_NAME_BLUR_
inline constexpr auto WIV_NAME_BLUR_USE_KEY{ "blr" };
#define WIV_NAME_BLUR_USE_VAL blur_use

inline constexpr auto WIV_NAME_BLUR_RADIUS_KEY{ "blr_r" };
#define WIV_NAME_BLUR_RADIUS_VAL blur_radius

inline constexpr auto WIV_NAME_BLUR_SIGMA_KEY{ "blr_s" };
#define WIV_NAME_BLUR_SIGMA_VAL blur_sigma

// WIV_NAME_UNSHARP_
inline constexpr auto WIV_NAME_UNSHARP_USE_KEY{ "shrp" };
#define WIV_NAME_UNSHARP_USE_VAL unsharp_use

inline constexpr auto WIV_NAME_UNSHARP_RADIUS_KEY{ "shrp_r" };
#define WIV_NAME_UNSHARP_RADIUS_VAL unsharp_radius

inline constexpr auto WIV_NAME_UNSHARP_SIGMA_KEY{ "shrp_s" };
#define WIV_NAME_UNSHARP_SIGMA_VAL unsharp_sigma

inline constexpr auto WIV_NAME_UNSHARP_AMOUNT_KEY{ "shrp_a" };
#define WIV_NAME_UNSHARP_AMOUNT_VAL unsharp_amount

// WIV_NAME_PASS_
inline constexpr auto WIV_NAME_PASS_FORMAT_KEY{ "pass_frmt" };
#define WIV_NAME_PASS_FORMAT_VAL pass_format

// WIV_NAME_RAW_
inline constexpr auto WIV_NAME_RAW_READ_THUMBNAIL_KEY{ "raw_tmb" };
#define WIV_NAME_RAW_READ_THUMBNAIL_VAL raw_thumb

//

struct Config_scale 
{
	// WIV_NAME_BLUR_
	bool WIV_NAME_BLUR_USE_VAL;
	int WIV_NAME_BLUR_RADIUS_VAL{ 2 };
	float WIV_NAME_BLUR_SIGMA_VAL{ 1.0f };

	// WIV_NAME_SIGMOID_
	float WIV_NAME_SIGMOID_CONTRAST_VAL{ 6.0f };
	float WIV_NAME_SIGMOID_MIDPOINT_VAL{ 0.6f };
	bool WIV_NAME_SIGMOID_USE_VAL;

	// WIV_NAME_KERNEL_
	int WIV_NAME_KERNEL_INDEX_VAL;
	float WIV_NAME_KERNEL_RADIUS_VAL{ 2.0f };
	float WIV_NAME_KERNEL_BLUR_VAL{ 1.0f };
	float WIV_NAME_KERNEL_PARAMETER1_VAL;
	float WIV_NAME_KERNEL_PARAMETER2_VAL;
	float WIV_NAME_KERNEL_ANTIRINGING_VAL{ 1.0f };
	bool WIV_NAME_KERNEL_USE_CYLINDRICAL_VAL;

	// WIV_NAME_UNSHARP_
	bool WIV_NAME_UNSHARP_USE_VAL;
	int WIV_NAME_UNSHARP_RADIUS_VAL{ 2 };
	float WIV_NAME_UNSHARP_SIGMA_VAL{ 1.0f };
	float WIV_NAME_UNSHARP_AMOUNT_VAL;
};

struct Scale_profile
{
	Range<float> range;
	Config_scale config;
};

class Config
{
public:
	void read();
	void write();

	// WIV_NAME_WINDOW_
	//

	// Client area.
	int WIV_NAME_WINDOW_WIDTH_VAL{ 1000 };
	int WIV_NAME_WINDOW_HEIGHT_VAL{ 618 };
	
	bool WIV_NAME_WINDOW_USE_AUTO_DIMS_VAL;
	int WIV_NAME_WINDOW_NAME_VAL;

	//

	// WIV_NAME_CLEAR_
	std::array<float, 4> WIV_NAME_CLEAR_COLOR_VAL{ 0.5f, 0.5f, 0.5f, 1.0f };
	
	// WIV_NAME_ALPHA_
	float WIV_NAME_ALPHA_TILE_SIZE_VAL{ 8.0f };
	std::array<float, 3> WIV_NAME_ALPHA_TILE1_COLOR_VAL{ 1.0f, 1.0f, 1.0f };
	std::array<float, 3> WIV_NAME_ALPHA_TILE2_COLOR_VAL{ 0.8f, 0.8f, 0.8f };

	// WIV_NAME_CMS_
	bool WIV_NAME_CMS_USE_VAL;
	int WIV_NAME_CMS_INTENT_VAL;
	bool WIV_NAME_CMS_USE_BLACKPOINT_COMPENSATION_VAL{ true };
	bool WIV_NAME_CMS_USE_DEFUALT_TO_SRGB_VAL;
	bool WIV_NAME_CMS_USE_DEFUALT_TO_ACES_VAL;
	int WIV_NAME_CMS_PROFILE_DISPLAY_VAL;
	std::filesystem::path WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM_VAL;
	int WIV_NAME_CMS_LUT_SIZE_VAL{ 33 };

	// WIV_NAME_PASS_
	int WIV_NAME_PASS_FORMAT_VAL;

	// WIV_NAME_RAW_
	bool WIV_NAME_RAW_READ_THUMBNAIL_VAL{ true };

	std::vector<Scale_profile> scale_profiles;
private:
	void read_top_level(const std::string& key, const std::string& val);
	void read_scale(const std::string& key, const std::string& val, Config_scale& scale);
	void write_top_level(std::ofstream& file);
	void write_scale(std::ofstream& file, const Config_scale& scale);
	std::filesystem::path get_path();
};

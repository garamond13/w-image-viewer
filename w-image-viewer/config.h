#pragma once

#include "pch.h"
#include "range.h"
#include "config_pairs.h"

enum WIV_COLOR_SPACE_
{
	WIV_COLOR_SPACE_NONE,
	WIV_COLOR_SPACE_SRGB,
	WIV_COLOR_SPACE_ADOBE,
	WIV_COLOR_SPACE_ACES,
	WIV_COLOR_SPACE_LINEAR_SRGB
};

enum WIV_CMS_PROFILE_DISPLAY_
{
	WIV_CMS_PROFILE_DISPLAY_AUTO,
	WIV_CMS_PROFILE_DISPLAY_SRGB,
	WIV_CMS_PROFILE_DISPLAY_ADOBE,
	WIV_CMS_PROFILE_DISPLAY_CUSTOM
};

enum WIV_WINDOW_NAME_
{
	WIV_WINDOW_NAME_DEFAULT,
	WIV_WINDOW_NAME_FILE_NAME,
	WIV_WINDOW_NAME_FILE_NAME_FULL
};

inline constexpr auto WIV_WINDOW_NAME{ L"W Image Viewer" };
inline constexpr auto WIV_WINDOW_STYLE{ WS_OVERLAPPEDWINDOW };
inline constexpr auto WIV_WINDOW_EX_STYLE{ WS_EX_ACCEPTFILES };

//window messages
inline constexpr auto WIV_WM_OPEN_FILE{ WM_USER + 0 };
inline constexpr auto WIV_WM_RESET_RESOURCES{ WM_USER + 1 };

inline constexpr std::array WIV_PASS_FORMATS{
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R16G16B16A16_FLOAT
};

struct Config_scale 
{
	//WIV_NAME_BLUR_
	bool WIV_NAME_BLUR_USE_VAL;
	int WIV_NAME_BLUR_RADIUS_VAL{ 2 };
	float WIV_NAME_BLUR_SIGMA_VAL{ 1.0f };

	//WIV_NAME_SIGMOID_
	float WIV_NAME_SIGMOID_CONTRAST_VAL{ 6.0f };
	float WIV_NAME_SIGMOID_MIDPOINT_VAL{ 0.6f };
	bool WIV_NAME_SIGMOID_USE_VAL;

	//WIV_NAME_KERNEL_
	int WIV_NAME_KERNEL_INDEX_VAL;
	float WIV_NAME_KERNEL_RADIUS_VAL{ 2.0f };
	float WIV_NAME_KERNEL_BLUR_VAL{ 1.0f };
	float WIV_NAME_KERNEL_PARAMETER1_VAL;
	float WIV_NAME_KERNEL_PARAMETER2_VAL;
	float WIV_NAME_KERNEL_ANTIRINGING_VAL{ 1.0f };
	bool WIV_NAME_KERNEL_USE_CYLINDRICAL_VAL;

	//WIV_NAME_UNSHARP_
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

	//WIV_NAME_WINDOW_
	int WIV_NAME_WINDOW_WIDTH_VAL{ 1000 };
	int WIV_NAME_WINDOW_HEIGHT_VAL{ 618 };
	bool WIV_NAME_WINDOW_USE_AUTO_DIMS_VAL;
	int WIV_NAME_WINDOW_NAME_VAL;

	//WIV_NAME_CLEAR_
	std::array<float, 4> WIV_NAME_CLEAR_COLOR_VAL{ 0.5f, 0.5f, 0.5f, 1.0f };
	
	//WIV_NAME_ALPHA_
	float WIV_NAME_ALPHA_TILE_SIZE_VAL{ 8.0f };
	std::array<float, 3> WIV_NAME_ALPHA_TILE1_COLOR_VAL{ 1.0f, 1.0f, 1.0f };
	std::array<float, 3> WIV_NAME_ALPHA_TILE2_COLOR_VAL{ 0.8f, 0.8f, 0.8f };

	//WIV_NAME_CMS_
	bool WIV_NAME_CMS_USE_VAL;
	int WIV_NAME_CMS_INTENT_VAL;
	bool WIV_NAME_CMS_USE_BLACKPOINT_COMPENSATION_VAL{ true };
	bool WIV_NAME_CMS_USE_DEFUALT_TO_SRGB_VAL;
	bool WIV_NAME_CMS_USE_DEFUALT_TO_ACES_VAL;
	int WIV_NAME_CMS_PROFILE_DISPLAY_VAL;
	std::filesystem::path WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM_VAL;

	//WIV_NAME_PASS_
	int WIV_NAME_PASS_FORMAT_VAL;

	//WIV_NAME_RAW_
	bool WIV_NAME_RAW_READ_THUMBNAIL_VAL{ true };

	std::vector<Scale_profile> scale_profiles;
private:
	void read_top_level(const std::string& key, const std::string& val);
	void read_scale(const std::string& key, const std::string& val, Config_scale& scale);
	void write_top_level(std::ofstream& file);
	void write_scale(std::ofstream& file, const Config_scale& scale);
	std::filesystem::path get_path();
};

#pragma once

#include "pch.h"

//names
//

//WIV_NAME_WINDOW_
#define WIV_NAME_WINDOW_WIDTH window_w
#define WIV_NAME_WINDOW_HEIGHT window_h
#define WIV_NAME_WINDOW_USE_AUTO_DIMS window_autowh
#define WIV_NAME_WINDOW_NAME window_name

//WIV_NAME_CLEAR_
#define WIV_NAME_CLEAR_COLOR clear_c

//WIV_NAME_ALPHA_
#define WIV_NAME_ALPHA_TILE1_COLOR alpha_t1_c
#define WIV_NAME_ALPHA_TILE2_COLOR alpha_t2_c
#define WIV_NAME_ALPHA_TILE_SIZE alpha_t_size

//WIV_NAME_KERNEL_
#define WIV_NAME_KERNEL_INDEX kernel_i
#define WIV_NAME_KERNEL_RADIUS kernel_r
#define WIV_NAME_KERNEL_BLUR kernel_b
#define WIV_NAME_KERNEL_PARAMETER1 kernel_p1
#define WIV_NAME_KERNEL_PARAMETER2 kernel_p2
#define WIV_NAME_KERNEL_ANTIRINGING kernel_ar
#define WIV_NAME_KERNEL_USE_CYLINDRICAL kernel_use_cyl

//WIV_NAME_SIGMOID_
#define WIV_NAME_SIGMOID_CONTRAST sigmoid_c
#define WIV_NAME_SIGMOID_MIDPOINT sigmoid_m
#define WIV_NAME_SIGMOID_USE sigmoid_use

//WIV_NAME_CMS_
#define WIV_NAME_CMS_USE cms_use
#define WIV_NAME_CMS_INTENT cms_intent
#define WIV_NAME_CMS_USE_BLACKPOINT_COMPENSATION cms_use_bpc
#define WIV_NAME_CMS_USE_DEFUALT_TO_SRGB cms_use_defualt_to_srgb
#define WIV_NAME_CMS_PROFILE_DISPLAY cms_profile_display //enum WIV_CMS_PROFILE_DISPLAY_
#define WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM cms_profile_display_custom

//WIV_NAME_BLUR_
#define WIV_NAME_BLUR_USE blur_use
#define WIV_NAME_BLUR_RADIUS blur_r
#define WIV_NAME_BLUR_SIGMA blur_s

//WIV_NAME_UNSHARP_
#define WIV_NAME_UNSHARP_USE unsharp_use
#define WIV_NAME_UNSHARP_RADIUS unsharp_r
#define WIV_NAME_UNSHARP_SIGMA unsharp_s
#define WIV_NAME_UNSHARP_AMOUNT unsharp_a

//WIV_NAME_PASS_
#define WIV_NAME_PASS_FORMAT pass_format

//dont use directly
#define wiv_nametostr_helper(x) #x

#define wiv_nametostr(x) wiv_nametostr_helper(x)

//

enum WIV_COLOR_SPACE_
{
	WIV_COLOR_SPACE_NONE,
	WIV_COLOR_SPACE_SRGB,
	WIV_COLOR_SPACE_ADOBE
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

inline constexpr auto WIV_SUPPORTED_FILE_TYPES{ L""
	"*.tif;*.tiff;*.tx;*.env;*.sm;*.vsm;" //tagged image file format (tiff) 
	"*.jpg;*jpeg;*.jpe;*.jif;*.jfif;*.jfi;" //joint photographic experts group (jpeg)
	"*.png;" //portable network graphics (png)
	"*.bmp;*.rle;*.dib;" //bitmap image (bmp)
	"*.exr;" //openEXR
	"*.pbm;*.pgm;*.ppm;*.pnm;" //pnm/netpbm
	"*.psd;" //psd
	"*.tga;*.icb;*.vda;*.vst;*.tpic;" //truevision tga (targa)
	"*.ico;" //icon
	"*.cr2;*.arw;*.raf;*.dng;*.nrw;*.nef;*.orf;*.rw2;" //raw
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

class Config
{
public:
	void file_read();
	void file_write();
	void values_map();
	void map_values();

	//WIV_NAME_WINDOW_
	int WIV_NAME_WINDOW_WIDTH;
	int WIV_NAME_WINDOW_HEIGHT;
	bool WIV_NAME_WINDOW_USE_AUTO_DIMS;
	int WIV_NAME_WINDOW_NAME;

	//WIV_NAME_CLEAR_
	std::array<float, 4> WIV_NAME_CLEAR_COLOR{ 0.0f, 0.0f, 0.0f, 1.0f };
	
	//WIV_NAME_ALPHA_
	float WIV_NAME_ALPHA_TILE_SIZE;
	std::array<float, 3> WIV_NAME_ALPHA_TILE1_COLOR;
	std::array<float, 3> WIV_NAME_ALPHA_TILE2_COLOR;
	
	//WIV_NAME_KERNEL_
	bool WIV_NAME_KERNEL_USE_CYLINDRICAL;
	int WIV_NAME_KERNEL_INDEX;
	float WIV_NAME_KERNEL_RADIUS;
	float WIV_NAME_KERNEL_BLUR;
	float WIV_NAME_KERNEL_PARAMETER1;
	float WIV_NAME_KERNEL_PARAMETER2;
	float WIV_NAME_KERNEL_ANTIRINGING;
	
	//WIV_NAME_SIGMOID_
	bool WIV_NAME_SIGMOID_USE;
	float WIV_NAME_SIGMOID_CONTRAST;
	float WIV_NAME_SIGMOID_MIDPOINT;

	//WIV_NAME_CMS_
	bool WIV_NAME_CMS_USE;
	int WIV_NAME_CMS_INTENT;
	bool WIV_NAME_CMS_USE_BLACKPOINT_COMPENSATION;
	bool WIV_NAME_CMS_USE_DEFUALT_TO_SRGB;
	int WIV_NAME_CMS_PROFILE_DISPLAY;
	std::filesystem::path WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM;

	//WIV_NAME_BLUR_
	bool WIV_NAME_BLUR_USE;
	int WIV_NAME_BLUR_RADIUS;
	float WIV_NAME_BLUR_SIGMA;

	//WIV_NAME_UNSHARP_
	bool WIV_NAME_UNSHARP_USE;
	int WIV_NAME_UNSHARP_RADIUS;
	float WIV_NAME_UNSHARP_SIGMA;
	float WIV_NAME_UNSHARP_AMOUNT;

	//WIV_NAME_PASS_
	int WIV_NAME_PASS_FORMAT;
private:
	std::filesystem::path get_path();
	std::unordered_map<std::string, std::string> map{

		//WIV_NAME_WINDOW_
		{ wiv_nametostr(WIV_NAME_WINDOW_WIDTH), "1000" },
		{ wiv_nametostr(WIV_NAME_WINDOW_HEIGHT), "618" },
		{ wiv_nametostr(WIV_NAME_WINDOW_USE_AUTO_DIMS), "0" },
		{ wiv_nametostr(WIV_NAME_WINDOW_NAME), "0" },

		//WIV_NAME_CLEAR_
		{ wiv_nametostr(WIV_NAME_CLEAR_COLOR[0]), "0.5" },
		{ wiv_nametostr(WIV_NAME_CLEAR_COLOR[1]), "0.5" },
		{ wiv_nametostr(WIV_NAME_CLEAR_COLOR[2]), "0.5" },

		//WIV_NAME_ALPHA_
		{ wiv_nametostr(WIV_NAME_ALPHA_TILE_SIZE), "8.0" },
		{ wiv_nametostr(WIV_NAME_ALPHA_TILE1_COLOR[0]), "1.0" },
		{ wiv_nametostr(WIV_NAME_ALPHA_TILE1_COLOR[1]), "1.0" },
		{ wiv_nametostr(WIV_NAME_ALPHA_TILE1_COLOR[2]), "1.0" },
		{ wiv_nametostr(WIV_NAME_ALPHA_TILE2_COLOR[0]), "0.8" },
		{ wiv_nametostr(WIV_NAME_ALPHA_TILE2_COLOR[1]), "0.8" },
		{ wiv_nametostr(WIV_NAME_ALPHA_TILE2_COLOR[2]), "0.8" },

		//WIV_NAME_KERNEL_
		{ wiv_nametostr(WIV_NAME_KERNEL_USE_CYLINDRICAL), "0" },
		{ wiv_nametostr(WIV_NAME_KERNEL_INDEX), "0" },
		{ wiv_nametostr(WIV_NAME_KERNEL_RADIUS), "2.0" },
		{ wiv_nametostr(WIV_NAME_KERNEL_BLUR), "1.0" },
		{ wiv_nametostr(WIV_NAME_KERNEL_PARAMETER1), "0.0" },
		{ wiv_nametostr(WIV_NAME_KERNEL_PARAMETER2), "0.0" },
		{ wiv_nametostr(WIV_NAME_KERNEL_ANTIRINGING), "1.0" },

		//WIV_NAME_SIGMOID_
		{ wiv_nametostr(WIV_NAME_SIGMOID_USE), "0" },
		{ wiv_nametostr(WIV_NAME_SIGMOID_CONTRAST), "6.0" },
		{ wiv_nametostr(WIV_NAME_SIGMOID_MIDPOINT), "0.6" },

		//WIV_NAME_CMS_
		{ wiv_nametostr(WIV_NAME_CMS_USE), "0" },
		{ wiv_nametostr(WIV_NAME_CMS_PROFILE_DISPLAY), "0" },
		{ wiv_nametostr(WIV_NAME_CMS_INTENT), "0" },
		{ wiv_nametostr(WIV_NAME_CMS_USE_BLACKPOINT_COMPENSATION), "1" },
		{ wiv_nametostr(WIV_NAME_CMS_USE_DEFUALT_TO_SRGB), "0" },
		{ wiv_nametostr(WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM), "" },

		//WIV_NAME_BLUR_
		{ wiv_nametostr(WIV_NAME_BLUR_USE), "0" },
		{ wiv_nametostr(WIV_NAME_BLUR_RADIUS), "2" },
		{ wiv_nametostr(WIV_NAME_BLUR_SIGMA), "1.0" },

		//WIV_NAME_UNSHARP_
		{ wiv_nametostr(WIV_NAME_UNSHARP_USE), "0" },
		{ wiv_nametostr(WIV_NAME_UNSHARP_RADIUS), "2" },
		{ wiv_nametostr(WIV_NAME_UNSHARP_SIGMA), "1.0" },
		{ wiv_nametostr(WIV_NAME_UNSHARP_AMOUNT), "0.5" },

		//WIV_NAME_PASS_
		{ wiv_nametostr(WIV_NAME_PASS_FORMAT), "0" }
	};
};

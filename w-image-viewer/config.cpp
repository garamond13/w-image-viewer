#include "pch.h"
#include "config.h"

#define toval(x,t) (x = std::sto ##t (map[wiv_nametostr_helper(x)]))

#define tomap(x) (map[wiv_nametostr_helper(x)] = std::to_string(x))

void Config::file_read()
{
	std::ifstream file(get_path());
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			auto pos{ line.find('=') };
			if (pos != std::string::npos) {
				std::string key = line.substr(0, pos);
				std::string value = line.substr(pos + 1);
				if (map.count(key))
					map[key] = value;
			}
		}
	}
	else
		file_write();
}

void Config::file_write()
{
	std::ofstream file(get_path());
	for (auto const& [key, value] : map)
		file << key << "=" << value << "\n";
}

void Config::values_map()
{
	//WIV_NAME_WINDOW_
	toval(WIV_NAME_WINDOW_WIDTH, i);
	toval(WIV_NAME_WINDOW_HEIGHT, i);
	toval(WIV_NAME_WINDOW_USE_AUTO_DIMS, i);

	//WIV_NAME_CLEAR_
	toval(WIV_NAME_CLEAR_COLOR[0], f);
	toval(WIV_NAME_CLEAR_COLOR[1], f);
	toval(WIV_NAME_CLEAR_COLOR[2], f);

	//WIV_NAME_ALPHA_
	toval(WIV_NAME_ALPHA_TILE_SIZE, f);
	toval(WIV_NAME_ALPHA_TILE1_COLOR[0], f);
	toval(WIV_NAME_ALPHA_TILE1_COLOR[1], f);
	toval(WIV_NAME_ALPHA_TILE1_COLOR[2], f);
	toval(WIV_NAME_ALPHA_TILE2_COLOR[0], f);
	toval(WIV_NAME_ALPHA_TILE2_COLOR[1], f);
	toval(WIV_NAME_ALPHA_TILE2_COLOR[2], f);

	//WIV_NAME_KERNEL_
	toval(WIV_NAME_KERNEL_USE_CYLINDRICAL, i);
	toval(WIV_NAME_KERNEL_INDEX, i);
	toval(WIV_NAME_KERNEL_RADIUS, f);
	toval(WIV_NAME_KERNEL_BLUR, f);
	toval(WIV_NAME_KERNEL_ANTIRINGING, f);
	toval(WIV_NAME_KERNEL_PARAMETER1, f);
	toval(WIV_NAME_KERNEL_PARAMETER2, f);
	
	//WIV_NAME_BLUR_
	toval(WIV_NAME_BLUR_USE, i);
	toval(WIV_NAME_BLUR_RADIUS, i);
	toval(WIV_NAME_BLUR_SIGMA, f);

	//WIV_NAME_UNSHARP_
	toval(WIV_NAME_UNSHARP_USE, i);
	toval(WIV_NAME_UNSHARP_RADIUS, i);
	toval(WIV_NAME_UNSHARP_SIGMA, f);
	toval(WIV_NAME_UNSHARP_AMOUNT, f);

	//WIV_NAME_SIGMOID_
	toval(WIV_NAME_SIGMOID_USE, i);
	toval(WIV_NAME_SIGMOID_CONTRAST, f);
	toval(WIV_NAME_SIGMOID_MIDPOINT, f);

	//WIV_NAME_CMS_
	toval(WIV_NAME_CMS_USE, i);
	toval(WIV_NAME_CMS_INTENT, i);
	toval(WIV_NAME_CMS_USE_BLACKPOINT_COMPENSATION, i);
	toval(WIV_NAME_CMS_PROFILE_DISPLAY, i);
	toval(WIV_NAME_CMS_USE_DEFUALT_TO_SRGB, i);
	WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM = map[wiv_nametostr(WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM)];
}

void Config::map_values()
{
	//WIV_NAME_WINDOW_
	tomap(WIV_NAME_WINDOW_WIDTH);
	tomap(WIV_NAME_WINDOW_HEIGHT);
	tomap(WIV_NAME_WINDOW_USE_AUTO_DIMS);

	//WIV_NAME_CLEAR_
	tomap(WIV_NAME_CLEAR_COLOR[0]);
	tomap(WIV_NAME_CLEAR_COLOR[1]);
	tomap(WIV_NAME_CLEAR_COLOR[2]);

	//WIV_NAME_ALPHA_
	tomap(WIV_NAME_ALPHA_TILE_SIZE);
	tomap(WIV_NAME_ALPHA_TILE1_COLOR[0]);
	tomap(WIV_NAME_ALPHA_TILE1_COLOR[1]);
	tomap(WIV_NAME_ALPHA_TILE1_COLOR[2]);
	tomap(WIV_NAME_ALPHA_TILE2_COLOR[0]);
	tomap(WIV_NAME_ALPHA_TILE2_COLOR[1]);
	tomap(WIV_NAME_ALPHA_TILE2_COLOR[2]);

	//WIV_NAME_KERNEL_
	tomap(WIV_NAME_KERNEL_USE_CYLINDRICAL);
	tomap(WIV_NAME_KERNEL_INDEX);
	tomap(WIV_NAME_KERNEL_RADIUS);
	tomap(WIV_NAME_KERNEL_BLUR);
	tomap(WIV_NAME_KERNEL_ANTIRINGING);
	tomap(WIV_NAME_KERNEL_PARAMETER1);
	tomap(WIV_NAME_KERNEL_PARAMETER2);

	//WIV_NAME_BLUR_
	tomap(WIV_NAME_BLUR_USE);
	tomap(WIV_NAME_BLUR_RADIUS);
	tomap(WIV_NAME_BLUR_SIGMA);

	//WIV_NAME_UNSHARP_
	tomap(WIV_NAME_UNSHARP_USE);
	tomap(WIV_NAME_UNSHARP_RADIUS);
	tomap(WIV_NAME_UNSHARP_SIGMA);
	tomap(WIV_NAME_UNSHARP_AMOUNT);

	//WIV_NAME_SIGMOID_
	tomap(WIV_NAME_SIGMOID_USE);
	tomap(WIV_NAME_SIGMOID_CONTRAST);
	tomap(WIV_NAME_SIGMOID_MIDPOINT);

	//WIV_NAME_CMS_
	tomap(WIV_NAME_CMS_USE);
	tomap(WIV_NAME_CMS_INTENT);
	tomap(WIV_NAME_CMS_USE_BLACKPOINT_COMPENSATION);
	tomap(WIV_NAME_CMS_PROFILE_DISPLAY);
	tomap(WIV_NAME_CMS_USE_DEFUALT_TO_SRGB);
	map[wiv_nametostr(WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM)] = WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM.string();
}

std::filesystem::path Config::get_path()
{
#ifdef NDEBUG

	//config path: %USERPROFILE%\AppData\Local\WImageViewer\config.txt

	wchar_t* local_app_data;
	SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local_app_data);
	std::filesystem::path path{ std::move(local_app_data) };

	//append %USERPROFILE%\AppData\Local with our directory
	//check first if it exists and if not, create it
	path /= L"WImageViewer";
	if (!std::filesystem::exists(path))
		std::filesystem::create_directory(path);
	
	//finaly make the full path of the config file
	return path /= L"config.txt";
#else
	return L"config.txt";
#endif
}

#include "pch.h"
#include "config.h"
#include "helpers.h"

/* config example

top_level_key=top_level_value
top_level_key=top_level_value
top_level_key=top_level_value
#section
##subsection
subsection_key=subsection_value
subsection_key=subsection_value
subsection_key=subsection_value
##end
##subsection
subsection_key=subsection_value
subsection_key=subsection_value
subsection_key=subsection_value
##end
#end
#section
section_key=section_value
section_key=section_value
section_key=section_value
#end

*/

void Config::read()
{
	bool is_section_scale{};
	Range<float> range;
	Config_scale scale;
	std::ifstream file(get_path());
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {

			//check for sections
			if (line[0] == '#' && line[1] != '#')
				if (line.substr(1) == "scale") {
					is_section_scale = true;
					continue;
				}
			
			//top level
			if (!is_section_scale) {
				auto pos{ line.find('=') };
				if (pos != std::string::npos) {
					read_top_level(line.substr(0, pos), line.substr(pos + 1));
				}
			}

			//section #scale
			if (is_section_scale) {

				//check for subsections
				if (line[0] == '#' && line[1] == '#') {
					if (line.substr(2) == "end")
						scale_profiles.push_back({range, scale});
					else {
						auto pos{ line.find(';') };
						strtoval(line.substr(2, pos), range.lower);
						strtoval(line.substr(pos + 1), range.upper);
					}
					continue;
				}

				auto pos{ line.find('=') };
				if (pos != std::string::npos)
					read_scale(line.substr(0, pos), line.substr(pos + 1), scale);
			}
		}	
	}
	else
		write();
}

void Config::write()
{
	//top level
	std::ofstream file(get_path());
	write_top_level(file);
	
	//scale
	file << "#scale\n";
	if(scale_profiles.empty())
		scale_profiles.push_back({});
	for (const auto& profile : scale_profiles) {
		file << "##" << std::to_string(profile.range.lower) + ";" + std::to_string(profile.range.upper) << '\n';
		write_scale(file, profile.config);
		file << "##end\n";
	}
	file << "#end\n";
}

void Config::read_top_level(const std::string& key, const std::string& val)
{
#ifdef read
#undef read
#endif
#define read(name) if (key == name ## _KEY) { strtoval(val, name ## _VAL); return; }

#ifdef read_array
#undef read_array
#endif
#define read_array(name,i) if (key == name ## _ ## i ## _KEY) { strtoval(val, name ## _VAL [i]); return; }

	//WIV_NAME_WINDOW_
	read(WIV_NAME_WINDOW_WIDTH)
	read(WIV_NAME_WINDOW_HEIGHT)
	read(WIV_NAME_WINDOW_USE_AUTO_DIMS)
	read(WIV_NAME_WINDOW_NAME)

	//WIV_NAME_CLEAR_
	read_array(WIV_NAME_CLEAR_COLOR, 0)
	read_array(WIV_NAME_CLEAR_COLOR, 1)
	read_array(WIV_NAME_CLEAR_COLOR, 2)

	//WIV_NAME_ALPHA_
	read(WIV_NAME_ALPHA_TILE_SIZE)
	read_array(WIV_NAME_ALPHA_TILE1_COLOR, 0)
	read_array(WIV_NAME_ALPHA_TILE1_COLOR, 1)
	read_array(WIV_NAME_ALPHA_TILE1_COLOR, 2)
	read_array(WIV_NAME_ALPHA_TILE2_COLOR, 0)
	read_array(WIV_NAME_ALPHA_TILE2_COLOR, 1)
	read_array(WIV_NAME_ALPHA_TILE2_COLOR, 2)

	//WIV_NAME_CMS_
	read(WIV_NAME_CMS_USE)
	read(WIV_NAME_CMS_INTENT)
	read(WIV_NAME_CMS_USE_BLACKPOINT_COMPENSATION)
	read(WIV_NAME_CMS_USE_DEFUALT_TO_SRGB)
	read(WIV_NAME_CMS_PROFILE_DISPLAY)
	if (key == WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM_KEY) {
		WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM_VAL = val;
		return;
	}

	//WIV_NAME_PASS_
	read(WIV_NAME_PASS_FORMAT)

	//WIV_NAME_RAW_
	read(WIV_NAME_RAW_READ_THUMBNAIL)
}

void Config::read_scale(const std::string& key, const std::string& val, Config_scale& scale)
{
#ifdef read
#undef read
#endif
#define read(name) if (key == name ## _KEY) { strtoval(val, scale.name ## _VAL); return; }

	//WIV_NAME_BLUR_
	read(WIV_NAME_BLUR_USE)
	read(WIV_NAME_BLUR_RADIUS)
	read(WIV_NAME_BLUR_SIGMA)

	//WIV_NAME_SIGMOID_
	read(WIV_NAME_SIGMOID_USE)
	read(WIV_NAME_SIGMOID_CONTRAST)
	read(WIV_NAME_SIGMOID_MIDPOINT)

	//WIV_NAME_KERNEL_
	read(WIV_NAME_KERNEL_USE_CYLINDRICAL)
	read(WIV_NAME_KERNEL_INDEX)
	read(WIV_NAME_KERNEL_RADIUS)
	read(WIV_NAME_KERNEL_BLUR)
	read(WIV_NAME_KERNEL_ANTIRINGING)
	read(WIV_NAME_KERNEL_PARAMETER1)
	read(WIV_NAME_KERNEL_PARAMETER2)

	//WIV_NAME_UNSHARP_
	read(WIV_NAME_UNSHARP_USE)
	read(WIV_NAME_UNSHARP_RADIUS)
	read(WIV_NAME_UNSHARP_SIGMA)
	read(WIV_NAME_UNSHARP_AMOUNT)
}

void Config::write_top_level(std::ofstream& file)
{
#ifdef write
#undef write
#endif
#define write(name) file << name ## _KEY << '=' << name ## _VAL << '\n';

#ifdef write_array
#undef write_array
#endif
#define write_array(name,i) file << name ## _ ## i ## _KEY << '=' << name ## _VAL [i] << '\n';

	//WIV_NAME_WINDOW_
	write(WIV_NAME_WINDOW_WIDTH)
	write(WIV_NAME_WINDOW_HEIGHT)
	write(WIV_NAME_WINDOW_USE_AUTO_DIMS)
	write(WIV_NAME_WINDOW_NAME)

	//WIV_NAME_CLEAR_
	write_array(WIV_NAME_CLEAR_COLOR, 0)
	write_array(WIV_NAME_CLEAR_COLOR, 1)
	write_array(WIV_NAME_CLEAR_COLOR, 2)

	//WIV_NAME_ALPHA_
	write(WIV_NAME_ALPHA_TILE_SIZE)
	write_array(WIV_NAME_ALPHA_TILE1_COLOR, 0)
	write_array(WIV_NAME_ALPHA_TILE1_COLOR, 1)
	write_array(WIV_NAME_ALPHA_TILE1_COLOR, 2)
	write_array(WIV_NAME_ALPHA_TILE2_COLOR, 0)
	write_array(WIV_NAME_ALPHA_TILE2_COLOR, 1)
	write_array(WIV_NAME_ALPHA_TILE2_COLOR, 2)

	//WIV_NAME_CMS_
	write(WIV_NAME_CMS_USE)
	write(WIV_NAME_CMS_INTENT)
	write(WIV_NAME_CMS_USE_BLACKPOINT_COMPENSATION)
	write(WIV_NAME_CMS_USE_DEFUALT_TO_SRGB)
	write(WIV_NAME_CMS_PROFILE_DISPLAY)
	file << WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM_KEY << '=' << WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM_VAL.string() << '\n';

	//WIV_NAME_PASS_
	write(WIV_NAME_PASS_FORMAT)

	//WIV_NAME_RAW_
	write(WIV_NAME_RAW_READ_THUMBNAIL)
}

void Config::write_scale(std::ofstream& file, const Config_scale& scale)
{
#ifdef write
#undef write
#endif
#define write(name) file << name ## _KEY << '=' << scale.name ## _VAL << '\n';

	//WIV_NAME_BLUR_
	write(WIV_NAME_BLUR_USE)
	write(WIV_NAME_BLUR_RADIUS)
	write(WIV_NAME_BLUR_SIGMA)

	//WIV_NAME_SIGMOID_
	write(WIV_NAME_SIGMOID_USE)
	write(WIV_NAME_SIGMOID_CONTRAST)
	write(WIV_NAME_SIGMOID_MIDPOINT)

	//WIV_NAME_KERNEL_
	write(WIV_NAME_KERNEL_USE_CYLINDRICAL)
	write(WIV_NAME_KERNEL_INDEX)
	write(WIV_NAME_KERNEL_RADIUS)
	write(WIV_NAME_KERNEL_BLUR)
	write(WIV_NAME_KERNEL_ANTIRINGING)
	write(WIV_NAME_KERNEL_PARAMETER1)
	write(WIV_NAME_KERNEL_PARAMETER2)

	//WIV_NAME_UNSHARP_
	write(WIV_NAME_UNSHARP_USE)
	write(WIV_NAME_UNSHARP_RADIUS)
	write(WIV_NAME_UNSHARP_SIGMA)
	write(WIV_NAME_UNSHARP_AMOUNT)
}

std::filesystem::path Config::get_path()
{
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(nullptr, path, MAX_PATH);
	return std::filesystem::path(path).parent_path() / L"config.txt";
}

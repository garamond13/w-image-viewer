#include "pch.h"
#include "config.h"

/* Config example

top_level_key=value
top_level_key=value
top_level_key=value0,value1,value2
#section
##subsection
subsection_key=value
subsection_key=value
subsection_key=value
##end
##subsection
subsection_key=value0,value1
subsection_key=value
subsection_key=value
##end
#end
#section
section_key=value
section_key=value
section_key=value
#end

*/

void Config::read()
{
	bool is_section_scale{};
	Right_open_range<float> range;
	Config_scale scale;
	std::ifstream file(get_path());
	scale_profiles.clear();
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {

			// Check for sections.
			if (line[0] == '#' && line[1] != '#')
				if (line.substr(1) == "scale") {
					is_section_scale = true;
					continue;
				}
			
			// Top level
			if (!is_section_scale) {
				auto pos{ line.find('=') };
				if (pos != std::string::npos) {
					read_top_level(line.substr(0, pos), line.substr(pos + 1));
				}
			}

			// Section #scale
			if (is_section_scale) {

				// Check for subsections.
				if (line[0] == '#' && line[1] == '#') {
					if (line.substr(2) == "end")
						scale_profiles.push_back({range, scale});
					else {
						auto pos{ line.find(',') };
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
	// Top level
	std::ofstream file(get_path());
	write_top_level(file);
	
	// Scale
	file << "#scale\n";
	if(scale_profiles.empty())
		scale_profiles.push_back({});
	for (const auto& profile : scale_profiles) {
		file << "##" << std::to_string(profile.range.lower) + "," + std::to_string(profile.range.upper) << '\n';
		write_scale(file, profile.config);
		file << "##end\n";
	}
	file << "#end\n";
}

void Config::read_top_level(const std::string& key, const std::string& val)
{

// Macro helpers for reading config.
//

#undef read
#define read(name)\
	if (key == name ## .key) {\
		strtoval(val, name ## .val);\
		return;\
	}

#undef read_array
#define read_array(name)\
	if (key == name ## .key) {\
		std::string sub_val;\
		std::stringstream ss{ val };\
		int i{};\
		while (std::getline(ss, sub_val, ',')) {\
			strtoval(sub_val, name ## .val[i]);\
			++i;\
		}\
		return;\
	}

//

	read(window_width)
	read(window_height)
	read(window_min_width)
	read(window_min_height)
	read(window_keep_aspect)
	read(window_autowh)
	read(window_autowh_center)
	read(window_name)
	read_array(clear_color)
	read(alpha_tile_size)
	read_array(alpha_tile1_color)
	read_array(alpha_tile2_color)
	read(cms_use)
	read(cms_intent)
	read(cms_bpc_use)
	read(cms_default_to_srgb)
	read(cms_default_to_aces)
	read(cms_display_profile)
	if (key == cms_display_profile_custom.key) {
		cms_display_profile_custom.val = val;
		return;
	}
	read(cms_lut_size)
	read(pass_format)
	read(raw_thumb)
	read(overlay_show)
	read(overlay_position)
	read(overlay_config)
}

void Config::read_scale(const std::string& key, const std::string& val, Config_scale& scale)
{

// Macro helper for reading config.
#undef read
#define read(name)\
	if (key == scale. ## name ## .key) {\
		strtoval(val, scale. ## name ## .val);\
		return;\
	}

	read(blur_use)
	read(blur_radius)
	read(blur_sigma)
	read(sigmoid_use)
	read(sigmoid_contrast)
	read(sigmoid_midpoint)
	read(kernel_cylindrical_use)
	read(kernel_index)
	read(kernel_radius)
	read(kernel_blur)
	read(kernel_antiringing)
	read(kernel_parameter1)
	read(kernel_parameter2)
	read(unsharp_use)
	read(unsharp_radius)
	read(unsharp_sigma)
	read(unsharp_amount)
}

void Config::write_top_level(std::ofstream& file)
{

// Macro helpers for writing config.
//

#undef write
#define write(name) file << name ## .key << '=' << name ## .val << '\n';

#undef write_array
#define write_array(name)\
	file << name ## .key << '=';\
	for (int i{}; i < name ## .val.size(); ++i) {\
		file << name ## .val[i];\
		if (i != name ## .val.size() - 1)\
			file << ',';\
	}\
	file << '\n';

//

	write(window_width)
	write(window_height)
	write(window_min_width)
	write(window_min_height)
	write(window_keep_aspect)
	write(window_autowh)
	write(window_autowh_center)
	write(window_name)
	write_array(clear_color)
	write(alpha_tile_size)
	write_array(alpha_tile1_color)
	write_array(alpha_tile2_color)
	write(cms_use)
	write(cms_intent)
	write(cms_bpc_use)
	write(cms_default_to_srgb)
	write(cms_default_to_aces)
	write(cms_display_profile)
	file << cms_display_profile_custom.key << '=' << cms_display_profile_custom.val.string() << '\n';
	write(cms_lut_size)
	write(pass_format)
	write(raw_thumb)
	write(overlay_show)
	write(overlay_position)
	write(overlay_config)
}

void Config::write_scale(std::ofstream& file, const Config_scale& scale)
{

// Macro helper for writing config.
#undef write
#define write(name) file << scale. ## name ## .key << '=' << scale. ## name ## .val << '\n';

	write(blur_use)
	write(blur_radius)
	write(blur_sigma)
	write(sigmoid_use)
	write(sigmoid_contrast)
	write(sigmoid_midpoint)
	write(kernel_cylindrical_use)
	write(kernel_index)
	write(kernel_radius)
	write(kernel_blur)
	write(kernel_antiringing)
	write(kernel_parameter1)
	write(kernel_parameter2)
	write(unsharp_use)
	write(unsharp_radius)
	write(unsharp_sigma)
	write(unsharp_amount)
}

std::filesystem::path Config::get_path()
{
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(nullptr, path, MAX_PATH);
	return std::filesystem::path(path).parent_path() / L"config";
}

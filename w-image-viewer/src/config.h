#pragma once

#include "pch.h"
#include "include\range.h"
#include "include\helpers.h"

template<typename T, Char_array str>
struct Config_pair
{
    T val;
    static constexpr const char* key = str.val.data();
};

struct Config_scale 
{
    Config_pair<bool, "bu"> blur_use;
    Config_pair<int, "br"> blur_radius = { 2 };
    Config_pair<float, "bs"> blur_sigma = { 1.0f };
    Config_pair<bool, "sigu"> sigmoid_use;
    Config_pair<float, "sigc"> sigmoid_contrast = { 6.0f };
    Config_pair<float, "sigm"> sigmoid_midpoint = { 0.6f };
    Config_pair<int, "ki"> kernel_index;
    Config_pair<float, "kr"> kernel_support = { 2.0f };
    Config_pair<float, "kb"> kernel_blur = { 1.0f };
    Config_pair<float, "kp1"> kernel_parameter1 = { 0.16f };
    Config_pair<float, "kp2"> kernel_parameter2 = { 1.0f };
    Config_pair<float, "ka"> kernel_antiringing = { 1.0f };
    Config_pair<bool, "kc"> kernel_cylindrical_use;
    Config_pair<bool, "usu"> unsharp_use;
    Config_pair<int, "usr"> unsharp_radius = { 2 };
    Config_pair<float, "uss"> unsharp_sigma = { 1.0f };
    Config_pair<float, "usa"> unsharp_amount;
};

struct Scale_profile
{
    Right_open_range<float> range;
    Config_scale config;
};

class Config
{
public:
    void read();
    void write();
    void read_slideshow();
    void write_slideshow();

    // Client area.
    Config_pair<int, "ww"> window_width = { 1300 };
    Config_pair<int, "wh"> window_height = { 803 };
    Config_pair<int, "wmw"> window_min_width = { 0 };
    Config_pair<int, "wmh"> window_min_height = { 0 };
    Config_pair<bool, "wka"> window_keep_aspect;
    
    Config_pair<bool, "waw"> window_autowh;
    Config_pair<bool, "wac"> window_autowh_center;
    Config_pair<int, "wn"> window_name = { 1 };
    Config_pair<std::array<float, 4>, "clrc"> clear_color = { 0.5f, 0.5f, 0.5f, 1.0f };
    Config_pair<float, "atsz"> alpha_tile_size = { 8.0 };
    Config_pair<std::array<float, 3>, "at1c"> alpha_tile1_color = { 1.0f, 1.0f, 1.0f };
    Config_pair<std::array<float, 3>, "at2c"> alpha_tile2_color = { 0.8f, 0.8f, 0.8f };
    Config_pair<bool, "cmu"> cms_use;
    Config_pair<int, "cmi"> cms_intent;
    Config_pair<bool, "cmb"> cms_bpc_use;
    Config_pair<bool, "cmds"> cms_default_to_srgb { true };
    Config_pair<bool, "cmda"> cms_default_to_aces;
    Config_pair<int, "cmdp"> cms_display_profile;
    Config_pair<std::filesystem::path, "cmdpc"> cms_display_profile_custom;
    Config_pair<unsigned int, "cmlsz"> cms_lut_size = { 33 };
    Config_pair<bool, "cmd"> cms_dither { true };
    Config_pair<int, "fmt"> pass_format;
    Config_pair<bool, "rwt"> raw_thumb = { true };
    std::vector<Scale_profile> scale_profiles;
    Config_pair<bool, "oshw"> overlay_show;
    Config_pair<int, "opos"> overlay_position;
    Config_pair<uint64_t, "ocfg"> overlay_config;
    Config_pair<bool, "cf"> cycle_files;
    Config_pair<bool, "ssac"> slideshow_auto_close;
    Config_pair<float, "ssi"> slideshow_interval = { 5.0f };
private:
    void read_top_level(const std::string& key, const std::string& val);
    void read_scale(const std::string& key, const std::string& val, Config_scale& scale);
    void write_top_level(std::ofstream& file);
    void write_scale(std::ofstream& file, const Config_scale& scale);
    std::filesystem::path get_path();
};

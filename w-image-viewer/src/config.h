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
    Config_pair<bool, "blr"> blur_use;
    Config_pair<int, "blr_r"> blur_radius = { 2 };
    Config_pair<float, "blr_s"> blur_sigma = { 1.0f };
    Config_pair<bool, "sig"> sigmoid_use;
    Config_pair<float, "sig_c"> sigmoid_contrast = { 6.0f };
    Config_pair<float, "sig_m"> sigmoid_midpoint = { 0.6f };
    Config_pair<int, "k_i"> kernel_index;
    Config_pair<float, "k_r"> kernel_support = { 2.0f };
    Config_pair<float, "k_b"> kernel_blur = { 1.0f };
    Config_pair<float, "k_p1"> kernel_parameter1 = { 0.16f };
    Config_pair<float, "k_p2"> kernel_parameter2 = { 1.0f };
    Config_pair<float, "k_ar"> kernel_antiringing = { 1.0f };
    Config_pair<bool, "k_cyl"> kernel_cylindrical_use;
    Config_pair<bool, "us"> unsharp_use;
    Config_pair<int, "us_r"> unsharp_radius = { 2 };
    Config_pair<float, "us_s"> unsharp_sigma = { 1.0f };
    Config_pair<float, "us_amt"> unsharp_amount;
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

    // Client area.
    Config_pair<int, "wnd_w"> window_width = { 1300 };
    Config_pair<int, "wnd_h"> window_height = { 803 };
    Config_pair<int, "wnd_mnw"> window_min_width = { 0 };
    Config_pair<int, "wnd_mnh"> window_min_height = { 0 };
    Config_pair<bool, "wnd_kpasp"> window_keep_aspect;
    
    Config_pair<bool, "wnd_autwh"> window_autowh;
    Config_pair<bool, "wnd_autwhc"> window_autowh_center;
    Config_pair<int, "wnd_name"> window_name = { 1 };
    Config_pair<std::array<float, 4>, "clr_c"> clear_color = { 0.5f, 0.5f, 0.5f, 1.0f };
    Config_pair<float, "a_tsz"> alpha_tile_size = { 8.0 };
    Config_pair<std::array<float, 3>, "a_t1c"> alpha_tile1_color = { 1.0f, 1.0f, 1.0f };
    Config_pair<std::array<float, 3>, "a_t2c"> alpha_tile2_color = { 0.8f, 0.8f, 0.8f };
    Config_pair<bool, "cms"> cms_use;
    Config_pair<int, "cms_intent"> cms_intent;
    Config_pair<bool, "cms_bpc"> cms_bpc_use;
    Config_pair<bool, "cms_deftosrgb"> cms_default_to_srgb;
    Config_pair<bool, "cms_deftoaces"> cms_default_to_aces;
    Config_pair<int, "cms_prof"> cms_display_profile;
    Config_pair<std::filesystem::path, "cms_prof_cus"> cms_display_profile_custom;
    Config_pair<unsigned int, "cms_lutsz"> cms_lut_size = { 33 };
    Config_pair<bool, "cms_dither"> cms_dither;
    Config_pair<int, "pass_fmt"> pass_format;
    Config_pair<bool, "raw_tmb"> raw_thumb = { true };
    std::vector<Scale_profile> scale_profiles;
    Config_pair<bool, "ovrl"> overlay_show;
    Config_pair<int, "ovrl_pos"> overlay_position;
    Config_pair<uint64_t, "ovrl_cfg"> overlay_config;
    Config_pair<bool, "cycle_files"> cycle_files;
private:
    void read_top_level(const std::string& key, const std::string& val);
    void read_scale(const std::string& key, const std::string& val, Config_scale& scale);
    void write_top_level(std::ofstream& file);
    void write_scale(std::ofstream& file, const Config_scale& scale);
    std::filesystem::path get_path();
};

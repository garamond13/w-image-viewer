#pragma once

#include "pch.h"

//WIV_NAME_WINDOW_
inline constexpr auto WIV_NAME_WINDOW_WIDTH_KEY{ "wnd_w" };
#define WIV_NAME_WINDOW_WIDTH_VAL window_width

inline constexpr auto WIV_NAME_WINDOW_HEIGHT_KEY{ "wnd_h" };
#define WIV_NAME_WINDOW_HEIGHT_VAL window_height

inline constexpr auto WIV_NAME_WINDOW_USE_AUTO_DIMS_KEY{ "wnd_autwh" };
#define WIV_NAME_WINDOW_USE_AUTO_DIMS_VAL window_autowh

inline constexpr auto WIV_NAME_WINDOW_NAME_KEY{ "wnd_name" };
#define WIV_NAME_WINDOW_NAME_VAL window_name

//WIV_NAME_CLEAR_
inline constexpr std::array WIV_NAME_CLEAR_COLOR_KEY{ "clr_cr", "clr_cg", "clr_cb" };
#define WIV_NAME_CLEAR_COLOR_VAL clear_color

//WIV_NAME_ALPHA_
inline constexpr std::array WIV_NAME_ALPHA_TILE1_COLOR_KEY{ "a_t1_cr", "a_t1_cg", "a_t1_cb" };
#define WIV_NAME_ALPHA_TILE1_COLOR_VAL alpha_tile1_color

inline constexpr std::array WIV_NAME_ALPHA_TILE2_COLOR_KEY{ "a_t2_cr", "a_t2_cg", "a_t2_cb" };
#define WIV_NAME_ALPHA_TILE2_COLOR_VAL alpha_tile2_color

inline constexpr auto WIV_NAME_ALPHA_TILE_SIZE_KEY{ "a_t_size" };
#define WIV_NAME_ALPHA_TILE_SIZE_VAL alpha_tile_size

//WIV_NAME_KERNEL_
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

//WIV_NAME_SIGMOID_
inline constexpr auto WIV_NAME_SIGMOID_USE_KEY{ "sig" };
#define WIV_NAME_SIGMOID_USE_VAL sigmoid_use

inline constexpr auto WIV_NAME_SIGMOID_CONTRAST_KEY{ "sig_c" };
#define WIV_NAME_SIGMOID_CONTRAST_VAL sigmoid_contrast

inline constexpr auto WIV_NAME_SIGMOID_MIDPOINT_KEY{ "sig_m" };
#define WIV_NAME_SIGMOID_MIDPOINT_VAL sigmoid_midpoint


//WIV_NAME_CMS_
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

inline constexpr auto WIV_NAME_CMS_PROFILE_DISPLAY_KEY{ "cms_prof" }; //enum WIV_CMS_PROFILE_DISPLAY_
#define WIV_NAME_CMS_PROFILE_DISPLAY_VAL cms_profile_display //enum WIV_CMS_PROFILE_DISPLAY_

inline constexpr auto WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM_KEY{ "cms_prof_cstm" };
#define WIV_NAME_CMS_PROFILE_DISPLAY_CUSTOM_VAL cms_profile_display_custom

//WIV_NAME_BLUR_
inline constexpr auto WIV_NAME_BLUR_USE_KEY{ "blr" };
#define WIV_NAME_BLUR_USE_VAL blur_use

inline constexpr auto WIV_NAME_BLUR_RADIUS_KEY{ "blr_r" };
#define WIV_NAME_BLUR_RADIUS_VAL blur_radius

inline constexpr auto WIV_NAME_BLUR_SIGMA_KEY{ "blr_s" };
#define WIV_NAME_BLUR_SIGMA_VAL blur_sigma

//WIV_NAME_UNSHARP_
inline constexpr auto WIV_NAME_UNSHARP_USE_KEY{ "shrp" };
#define WIV_NAME_UNSHARP_USE_VAL unsharp_use

inline constexpr auto WIV_NAME_UNSHARP_RADIUS_KEY{ "shrp_r" };
#define WIV_NAME_UNSHARP_RADIUS_VAL unsharp_radius

inline constexpr auto WIV_NAME_UNSHARP_SIGMA_KEY{ "shrp_s" };
#define WIV_NAME_UNSHARP_SIGMA_VAL unsharp_sigma

inline constexpr auto WIV_NAME_UNSHARP_AMOUNT_KEY{ "shrp_a" };
#define WIV_NAME_UNSHARP_AMOUNT_VAL unsharp_amount

//WIV_NAME_PASS_
inline constexpr auto WIV_NAME_PASS_FORMAT_KEY{ "pass_frmt" };
#define WIV_NAME_PASS_FORMAT_VAL pass_format

//WIV_NAME_RAW_
inline constexpr auto WIV_NAME_RAW_READ_THUMBNAIL_KEY{ "raw_tmb" };
#define WIV_NAME_RAW_READ_THUMBNAIL_VAL raw_thumb
#ifndef __SHADER_CONFIG_H__
#define __SHADER_CONFIG_H__

//this file is shared by both hlsl and cpp

#define WIV_CMS_LUT_SIZE 65

//enum WIV_CMS_TRC_
#define WIV_CMS_TRC_NONE 0
#define WIV_CMS_TRC_GAMMA 1
#define WIV_CMS_TRC_SRGB 2

//enum WIV_KERNEL_FUNCTION_
#define WIV_KERNEL_FUNCTION_LANCZOS 0
#define WIV_KERNEL_FUNCTION_GINSENG 1
#define WIV_KERNEL_FUNCTION_HAMMING 2
#define WIV_KERNEL_FUNCTION_POW_COSINE 3
#define WIV_KERNEL_FUNCTION_GARAMOND_KAISER 4
#define WIV_KERNEL_FUNCTION_POW_GARAMOND 5
#define WIV_KERNEL_FUNCTION_POW_BLACKMAN 6
#define WIV_KERNEL_FUNCTION_GNW 7
#define WIV_KERNEL_FUNCTION_SAID 8
#define WIV_KERNEL_FUNCTION_NEAREST 9
#define WIV_KERNEL_FUNCTION_LINEAR 10
#define WIV_KERNEL_FUNCTION_BICUBIC 11
#define WIV_KERNEL_FUNCTION_FSR 12
#define WIV_KERNEL_FUNCTION_BCSPLINE 13

#endif // __SHADER_CONFIG_H__
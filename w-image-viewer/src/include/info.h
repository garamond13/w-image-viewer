#pragma once

#include "pch.h"

// Use for collecting various info across the entire application.
struct Info
{
    Info() = delete;
    static inline int image_width; // The original image width.
    static inline int image_height; // The original image height.
    static inline float scale; // Current image scale.
    static inline int scaled_width; // Scaled image width.
    static inline int scaled_height; // Scaled image height.
    static inline std::string scale_filter; // Scale filter. 0 - orthogonal, 1 - cylindrical 
    static inline int kernel_index; // Currently used kernel function.
    static inline float kernel_support; // Valid range of the kernel function.
    static inline int kernel_radius; // Currently used kernel radius: ceil(kernel_support / min(scale, 1)).
    static inline uint8_t image_bitdepth; // Image bitdepth per channel.
    static inline uint8_t image_nchannels; // Number of channels per pixel.

    // Currently used scale kernel size.
    // In case of orthogonal scaling: kernel_radius * 2.
    // In case of cylindrical scaling: (kernel_radius * 2)^2.
    static inline int kernel_size;
};
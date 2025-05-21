#pragma once

#include "pch.h"

// Use for collecting various info across the entire application.
namespace info
{
	inline int image_width; // The original image width.
	inline int image_height; // The original image height.
	inline float scale; // Current image scale.
	inline int scaled_width; // Scaled image width.
	inline int scaled_height; // Scaled image height.
	inline std::string scale_filter; // Scale filter. 0 - orthogonal, 1 - cylindrical 
	inline int kernel_index; // Currently used kernel function.
	inline float kernel_support; // Valid range of the kernel function.
	inline int kernel_radius; // Currently used kernel radius: ceil(kernel_support / min(scale, 1)).
	inline uint8_t image_bitdepth; // Image bitdepth per channel.
	inline uint8_t image_nchannels; // Number of channels per pixel.

	// Currently used scale kernel size.
	// In case of orthogonal scaling: kernel_radius * 2.
	// In case of cylindrical scaling: (kernel_radius * 2)^2.
	inline int kernel_size;
};
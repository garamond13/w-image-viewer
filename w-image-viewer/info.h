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
	inline int kernel_index; // Currently used scale kernel.
	inline float kernel_radius; // Currently used kernel radius
	inline uint8_t image_bitdepth; // Image bitdepth per channel.
	inline uint8_t image_nchannels; // Number of channels per pixel.

	// Currently used scale kernel size.
	// In case of orthogonal scaling: (ceil(kernel_radius / scale) * 2) * 2.
	// In case of cylindrical scaling: (ceil(kernel_radius / scale) * 2) ^ 2.
	inline int kernel_size;
};
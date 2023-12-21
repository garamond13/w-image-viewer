#pragma once

// Use for collecting various info across the entire application.
struct Info
{
	int image_width; // The original image width.
	int image_height; // The original image height.
	float scale; // Current image scale.
	int scaled_width; // Scaled image width.
	int scaled_height; // Scaled image height.
	int kernel_index; // Currently used scale kernel.

	// Currently used scale kernel size.
	// In case of orthogonal scaling: (ceil(kernel_radius / scale) * 2) * 2.
	// In case of cylindrical scaling: (ceil(kernel_radius / scale) * 2) ^ 2.
	int kernel_size;
};
#pragma once

#include "pch.h"
#include "trc.h"

enum WIV_COLOR_SPACE_
{
	WIV_COLOR_SPACE_NONE,
	WIV_COLOR_SPACE_SRGB,
	WIV_COLOR_SPACE_ADOBE,
	WIV_COLOR_SPACE_ACES,
	WIV_COLOR_SPACE_LINEAR_SRGB
};

class Image
{
public:
	void get_data_for_d3d(std::unique_ptr<uint8_t[]>& data, DXGI_FORMAT& format, UINT& sys_mem_pitch);
	bool has_alpha() const noexcept;
	bool set_image_input(std::wstring_view path);
	bool close() noexcept;
	
	template<typename T>
	T get_width() const noexcept
	{
		return static_cast<T>(image_input->spec().width);
	}

	template<typename T>
	T get_height() const noexcept
	{
		return static_cast<T>(image_input->spec().height);
	}

	std::unique_ptr<std::remove_pointer_t<cmsHPROFILE>, decltype(&cmsCloseProfile)> embended_profile{ nullptr, cmsCloseProfile };
	int orientation;
	Tone_response_curve trc;
private:
	void get_embended_profile();

	template<typename T>
	std::unique_ptr<uint8_t[]> read_image()
	{
		// size = width * height * nchannels * bytedepth
		auto data{ std::make_unique_for_overwrite<uint8_t[]>(image_input->spec().width * image_input->spec().height * 4 * sizeof(T)) };

		image_input->read_image(0, 0, 0, -1, image_input->spec().format, data.get(), 4 * sizeof(T));

		// Convert a single channel greyscale image into multy channel greyscale image.
		switch (image_input->spec().nchannels) {
			case 1: // (grey null null null) into (grey grey grey null).
				for (int i{}; i < image_input->spec().width * image_input->spec().height; ++i)
					reinterpret_cast<T*>(data.get())[4 * i + 2] = reinterpret_cast<T*>(data.get())[4 * i + 1] = reinterpret_cast<T*>(data.get())[4 * i];
				break;
			case 2: // (grey alpha null null) into (grey grey grey alpha)
				for (int i{}; i < image_input->spec().width * image_input->spec().height; ++i) {
					reinterpret_cast<T*>(data.get())[4 * i + 3] = reinterpret_cast<T*>(data.get())[4 * i + 1];
					reinterpret_cast<T*>(data.get())[4 * i + 2] = reinterpret_cast<T*>(data.get())[4 * i + 1] = reinterpret_cast<T*>(data.get())[4 * i];
				}
		}
		
		// At this point we dont need raw_input data anymore.
		raw_input.recycle();
		
		return data;
	}
	
	std::unique_ptr<OIIO::ImageInput> image_input;
	LibRaw raw_input;
	OIIO::Filesystem::IOMemReader thumb{ nullptr, 0 };
};

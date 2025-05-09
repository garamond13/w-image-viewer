#pragma once

#include "pch.h"
#include "trc.h"
#include "info.h"

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
	bool is_valid() const noexcept;
	bool has_alpha() const noexcept;
	bool set_image_input(std::wstring_view path);
	bool close() noexcept;
	
	// OIIO::TypeDesc::BASETYPE
	auto get_base_type() const noexcept
	{
		return image_input->spec().format.basetype;
	}
	
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

	template<std::floating_point T>
	T get_aspect() const noexcept
	{
		return get_width<T>() / get_height<T>();
	}

	template<typename T>
	std::unique_ptr<uint8_t[]> read_image()
	{
		const auto& spec = image_input->spec();

		// size = width * height * nchannels * bytedepth
		auto data = std::make_unique_for_overwrite<uint8_t[]>(spec.width * spec.height * 4 * sizeof(T));

		image_input->read_image(0, 0, 0, -1, spec.format, data.get(), 4 * sizeof(T));

		// Convert a single channel greyscale image into multy channel greyscale image.
		switch (spec.nchannels) {
			case 1: // (grey null null null) into (grey grey grey null).
				for (int i = 0; i < spec.width * spec.height; ++i)
					reinterpret_cast<T*>(data.get())[4 * i + 2] = reinterpret_cast<T*>(data.get())[4 * i + 1] = reinterpret_cast<T*>(data.get())[4 * i];
				break;
			case 2: // (grey alpha null null) into (grey grey grey alpha)
				for (int i = 0; i < spec.width * spec.height; ++i) {
					reinterpret_cast<T*>(data.get())[4 * i + 3] = reinterpret_cast<T*>(data.get())[4 * i + 1];
					reinterpret_cast<T*>(data.get())[4 * i + 2] = reinterpret_cast<T*>(data.get())[4 * i + 1] = reinterpret_cast<T*>(data.get())[4 * i];
				}
		}
		
		// At this point we dont need raw_input data anymore.
		raw_input.recycle();
		
		info::image_bitdepth = spec.channel_bytes() * 8;
		info::image_nchannels = spec.nchannels;
		return data;
	}

	std::unique_ptr<std::remove_pointer_t<cmsHPROFILE>, decltype(&cmsCloseProfile)> embended_profile = { nullptr, cmsCloseProfile };
	int orientation;
	Tone_response_curve trc;
private:
	void get_embended_profile();
	std::unique_ptr<OIIO::ImageInput> image_input;
	LibRaw raw_input;
	OIIO::Filesystem::IOMemReader thumb = { nullptr, 0 };
};

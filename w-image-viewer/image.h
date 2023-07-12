#pragma once

#include "pch.h"

class Image
{
public:
	std::unique_ptr<uint8_t[]> get_data(DXGI_FORMAT& format, UINT& sys_mem_pitch);
	bool has_alpha() const noexcept;
	int get_tagged_color_space();
	bool set_image_input(std::wstring_view path);
	
	template<typename T>
	const T get_width() const noexcept
	{
		return static_cast<T>(image_input->spec().width);
	}

	template<typename T>
	const T get_height() const noexcept
	{
		return static_cast<T>(image_input->spec().height);
	}

	std::unique_ptr<std::remove_pointer_t<cmsHPROFILE>, decltype(&cmsCloseProfile)> embended_profile{ nullptr, cmsCloseProfile };
	int orientation;
private:
	cmsHPROFILE get_embended_profile();

	template<typename T>
	std::unique_ptr<uint8_t[]> read_image()
	{
		//size = width * height * nchannels * bytedepth
		auto data{ std::make_unique_for_overwrite<uint8_t[]>(image_input->spec().width * image_input->spec().height * 4 * sizeof(T)) };

		image_input->read_image(0, 0, 0, -1, image_input->spec().format, data.get(), 4 * sizeof(T));

		switch (image_input->spec().nchannels) {

			//convert single channel greyscale image into a 3 channel greyscale image
			//channels layout: (grey null null null) into (grey grey grey null)
		case 1:
			for (int i{}; i < image_input->spec().width * image_input->spec().height; ++i)
				reinterpret_cast<T*>(data.get())[4 * i + 2] = reinterpret_cast<T*>(data.get())[4 * i + 1] = reinterpret_cast<T*>(data.get())[4 * i];
			break;

			//convert single channel greyscale image + alpha into a 3 channel greyscale image + alpha
			//channels layout: (grey alpha null null) into (grey grey grey alpha)
		case 2:
			for (int i{}; i < image_input->spec().width * image_input->spec().height; ++i) {
				reinterpret_cast<T*>(data.get())[4 * i + 3] = reinterpret_cast<T*>(data.get())[4 * i + 1];
				reinterpret_cast<T*>(data.get())[4 * i + 2] = reinterpret_cast<T*>(data.get())[4 * i + 1] = reinterpret_cast<T*>(data.get())[4 * i];
			}
		}
		
		//at this point we dont need raw_input data anymore
		raw_input.recycle();
		
		return data;
	}
	
	std::unique_ptr<OIIO::ImageInput> image_input;
	LibRaw raw_input;
	OIIO::Filesystem::IOMemReader thumb{nullptr, 0};
};

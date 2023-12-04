#include "pch.h"
#include "image.h"
#include "global.h"

// Get image data, apropriate DXGI format and System-memory pitch for creation of d3d texture.
void Image::get_data_for_d3d(std::unique_ptr<uint8_t[]>& data, DXGI_FORMAT& format, UINT& sys_mem_pitch)
{
	switch (image_input->spec().format.basetype) {
		case OIIO::TypeDesc::UINT8:
			data = read_image<uint8_t>();
			format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sys_mem_pitch = image_input->spec().width * 4;
			break;
		case OIIO::TypeDesc::UINT16:
			data = read_image<uint16_t>();
			format = DXGI_FORMAT_R16G16B16A16_UNORM;
			sys_mem_pitch = image_input->spec().width * 4 * 2;
			break;
		case OIIO::TypeDesc::HALF:
			data = read_image<uint16_t>();
			format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			sys_mem_pitch = image_input->spec().width * 4 * 2;
			break;
		case OIIO::TypeDesc::FLOAT:
			data = read_image<uint32_t>();
			format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			sys_mem_pitch = image_input->spec().width * 4 * 4;
	}
}

bool Image::has_alpha() const noexcept
{
	return image_input->spec().alpha_channel != -1;
}

int Image::get_tagged_color_space()
{
	// OIIO should manage memory pointed by tag.
	const char* tag;
	
	const auto atribute_type{ image_input->spec().getattributetype("oiio:ColorSpace") };
	if (image_input->spec().getattribute("oiio:ColorSpace", atribute_type, &tag)) {

		// Note that std::strstr(const char* str, const char* strSearch)
		// returns a pointer to the first occurrence of strSearch in str, or nullptr if strSearch doesn't appear in str.

		if (std::strstr(tag, "sRGB"))
			return WIV_COLOR_SPACE_SRGB;
		if (std::strstr(tag, "AdobeRGB"))
			return WIV_COLOR_SPACE_ADOBE;

		// Checking for upper case "Linear" and lower case "linear". This should cover all cases.
		if (std::strstr(tag, "inear") /* Not typo. */ ) {
			if (g_config.cms_use_default_to_aces)
				return WIV_COLOR_SPACE_ACES;
			else
				return WIV_COLOR_SPACE_LINEAR_SRGB;
		}

		if (std::strstr(tag, "ACEScg"))
			return WIV_COLOR_SPACE_ACES;
		if (std::strstr(tag, "lin_srgb"))
			return WIV_COLOR_SPACE_LINEAR_SRGB;
	}
	if (g_config.cms_use_defualt_to_srgb)
		return WIV_COLOR_SPACE_SRGB;
	return WIV_COLOR_SPACE_NONE;
}

bool Image::set_image_input(std::wstring_view path)
{
	// First try to open file with libraw, since OIIO cant read thumbnails.
	if (g_config.raw_thumb && raw_input.open_file(path.data()) == LIBRAW_SUCCESS) {
		if (raw_input.unpack_thumb() == LIBRAW_SUCCESS) {
			thumb = { raw_input.imgdata.thumbnail.thumb, raw_input.imgdata.thumbnail.tlength };
			
			// OIIO::ImageInput::open(): The filename here is irelevant, we only need extension.
			if (raw_input.imgdata.thumbnail.tformat == LIBRAW_THUMBNAIL_JPEG) {
				image_input = OIIO::ImageInput::open(".jpg", nullptr, &thumb);
			}
			else // Try bmp.
				image_input = OIIO::ImageInput::open(".bmp", nullptr, &thumb);

			orientation = raw_input.imgdata.sizes.flip;
		}
		else
			image_input = OIIO::ImageInput::open(path.data());
	}
	else {
		OIIO::ImageSpec config;
		config["bmp:monochrome_detect"] = 0;
		image_input = OIIO::ImageInput::open(path.data(), &config);
	}
	if (!image_input)
		return false;
	embended_profile.reset(get_embended_profile());
	return true;
}

cmsHPROFILE Image::get_embended_profile()
{
	// Source https://www.color.org/technotes/ICC-Technote-ProfileEmbedding.pdf
	constexpr auto size{ 16'707'345 };

	auto buffer{ std::make_unique_for_overwrite<uint8_t[]>(size) };
	const auto attribute_type{ image_input->spec().getattributetype("ICCProfile") };
	if (image_input->spec().getattribute("ICCProfile", attribute_type, buffer.get()))
		return cmsOpenProfileFromMem(buffer.get(), size);
	return nullptr;
}

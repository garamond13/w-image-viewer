#include "pch.h"
#include "image.h"
#include "config.h"


std::unique_ptr<uint8_t[]> Image::get_data(DXGI_FORMAT& format, UINT& sys_mem_pitch)
{
	switch (image_input->spec().format.basetype) {
	case OIIO::TypeDesc::UINT8:
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sys_mem_pitch = image_input->spec().width * 4;
		return read_image<uint8_t>();
	case OIIO::TypeDesc::UINT16:
		format = DXGI_FORMAT_R16G16B16A16_UNORM;
		sys_mem_pitch = image_input->spec().width * 4 * 2;
		return read_image<uint16_t>();
	case OIIO::TypeDesc::HALF:
		format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		sys_mem_pitch = image_input->spec().width * 4 * 2;
		return read_image<uint16_t>();
	case OIIO::TypeDesc::FLOAT:
		format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		sys_mem_pitch = image_input->spec().width * 4 * 4;
		return read_image<uint32_t>();
	}
}

bool Image::has_alpha() const noexcept
{
	return image_input->spec().alpha_channel != -1;
}

int Image::get_tagged_color_space()
{
	//oiio should manage memory pointed by tag
	const char* tag;
	
	auto atribute_type{ image_input->spec().getattributetype("oiio:ColorSpace") };
	if (image_input->spec().getattribute("oiio:ColorSpace", atribute_type, &tag)) {

		//std::strstr(const char* str, const char* strSearch)
		//returns a pointer to the first occurrence of strSearch in str, or nullptr if strSearch doesn't appear in str
		if (std::strstr(tag, "sRGB"))
			return WIV_COLOR_SPACE_SRGB;
		else if (std::strstr(tag, "AdobeRGB"))
			return WIV_COLOR_SPACE_ADOBE;
	}

	//indicate failure
	return -1;
}

bool Image::set_image_input(std::wstring_view path)
{
	image_input = OIIO::ImageInput::open(path.data());
	if (!image_input)
		return false;
	embended_profile.reset(get_embended_profile());
	return true;
}

cmsHPROFILE Image::get_embended_profile()
{
	//source https://www.color.org/technotes/ICC-Technote-ProfileEmbedding.pdf
	constexpr auto size{ 16'707'345 };

	auto buffer{ std::make_unique_for_overwrite<uint8_t[]>(size) };
	auto attribute_type{ image_input->spec().getattributetype("ICCProfile") };
	if (image_input->spec().getattribute("ICCProfile", attribute_type, buffer.get()))
		return cmsOpenProfileFromMem(buffer.get(), size);
	return nullptr;
}

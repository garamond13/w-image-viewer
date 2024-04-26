#include "pch.h"
#include "image.h"
#include "global.h"
#include "icc.h"
#include "shader_config.h"

bool Image::isn_null() const noexcept
{
	return image_input.get();
}

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

bool Image::set_image_input(std::wstring_view path)
{
	// First try to open file with libraw, since OIIO cant read thumbnails.
	if (g_config.raw_thumb.val && raw_input.open_file(path.data()) == LIBRAW_SUCCESS) {
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
	get_embended_profile();
	return true;
}

bool Image::close() noexcept
{
	if (!image_input)
		return false;
	image_input.reset();
	return true;
}

// Resets member embended_profile and sets member trc.
void Image::get_embended_profile()
{
	// First try to get an embended ICC profile.
	//
	
	// Source https://www.color.org/technotes/ICC-Technote-ProfileEmbedding.pdf
	constexpr auto size{ 16'707'345 };

	auto buffer{ std::make_unique_for_overwrite<uint8_t[]>(size) };
	if (image_input->spec().getattribute("ICCProfile", image_input->spec().getattributetype("ICCProfile"), buffer.get())) {
		embended_profile.reset(cmsOpenProfileFromMem(buffer.get(), size));
		auto gamma{ static_cast<float>(cmsDetectRGBProfileGamma(embended_profile.get(), 0.1))};
		if (gamma < 0.0f) // On Error.
			trc = { WIV_CMS_TRC_NONE, 0.0f };
		else
			trc = { WIV_CMS_TRC_GAMMA, gamma };
		return;
	}

	//

	// If we don't have an embended ICC profile try to get a color tag.
	//   

	// OIIO should manage memory pointed by the tag.
	const char* tag;

	if (image_input->spec().getattribute("oiio:ColorSpace", image_input->spec().getattributetype("oiio:ColorSpace"), &tag)) {

		// Note that std::strstr(const char* str, const char* strSearch)
		// returns a pointer to the first occurrence of strSearch in str, or nullptr if strSearch doesn't appear in str.

		if (std::strstr(tag, "sRGB")) {
			embended_profile.reset(cmsCreate_sRGBProfile());
			trc = { WIV_CMS_TRC_SRGB, 0.0f };
			return;
		}
		if (std::strstr(tag, "AdobeRGB")) {
			embended_profile.reset(cms_create_profile_adobe_rgb());
			trc = { WIV_CMS_TRC_GAMMA, ADOBE_RGB_GAMMA<float> };
			return;
		}

		// Checking for upper case "Linear", lower case "linear" and "scene_linear".
		// This should cover all cases.
		if (std::strstr(tag, "inear") /* Not typo. */) {
			if (g_config.cms_default_to_aces.val)
				embended_profile.reset(cms_create_profile_aces_cg());
			else
				embended_profile.reset(cms_create_profile_linear_srgb());
			trc = { WIV_CMS_TRC_LINEAR, 0.0f };
			return;
		}

		if (std::strstr(tag, "ACEScg")) {
			embended_profile.reset(cms_create_profile_aces_cg());
			trc = { WIV_CMS_TRC_LINEAR, 0.0f };
			return;
		}
		if (std::strstr(tag, "lin_srgb")) {
			embended_profile.reset(cms_create_profile_linear_srgb());
			trc = { WIV_CMS_TRC_LINEAR, 0.0f };
			return;
		}
	}

	//

	if (g_config.cms_default_to_srgb.val) {
		embended_profile.reset(cmsCreate_sRGBProfile());
		trc = { WIV_CMS_TRC_SRGB, 0.0f };
	}
	else {
		embended_profile.reset();
		trc = { WIV_CMS_TRC_NONE, 0.0f };
	}
}

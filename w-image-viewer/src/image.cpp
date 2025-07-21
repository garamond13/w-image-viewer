#include "pch.h"
#include "image.h"
#include "include\global.h"
#include "icc.h"
#include "include\shader_config.h"

bool Image::is_valid() const noexcept
{
    return image_input.get();
}

bool Image::has_alpha() const noexcept
{
    return image_input->spec().alpha_channel != -1;
}

bool Image::open_image(const std::filesystem::path& path)
{
    // First try to open file with libraw, since OIIO cant read thumbnails.
    // If Config::raw_thumb is enabled.
    if (g_config.raw_thumb.val && raw_input.open_file(path.c_str()) == LIBRAW_SUCCESS && raw_input.unpack_thumb() == LIBRAW_SUCCESS) {
        // We still want to open extracted thumbnail with OIIO.
        OIIO::Filesystem::IOMemReader thumb(raw_input.imgdata.thumbnail.thumb, raw_input.imgdata.thumbnail.tlength);
        if (raw_input.imgdata.thumbnail.tformat == LIBRAW_THUMBNAIL_JPEG) {
            // The filename here is irelevant, we only need the extension.
            image_input = OIIO::ImageInput::open(".jpg", nullptr, &thumb);
        }
        else { // BMP.
            // The filename here is irelevant, we only need the extension.
            image_input = OIIO::ImageInput::open(".bmp", nullptr, &thumb);
        }

        orientation = raw_input.imgdata.sizes.flip;
    }
    else {
        OIIO::ImageSpec config;
        config["bmp:monochrome_detect"] = 0;
        image_input = OIIO::ImageInput::open(path, &config);
    }
    if (!image_input) {
        return false;
    }
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
    constexpr auto size = 16'707'345;

    auto buffer = std::make_unique_for_overwrite<uint8_t[]>(size);
    if (image_input->spec().getattribute("ICCProfile", image_input->spec().getattributetype("ICCProfile"), buffer.get())) {
        embended_profile.reset(cmsOpenProfileFromMem(buffer.get(), size));
        auto gamma = static_cast<float>(cmsDetectRGBProfileGamma(embended_profile.get(), 0.1));
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

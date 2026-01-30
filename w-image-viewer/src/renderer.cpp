#include "pch.h"
#include "renderer.h"
#include "include\global.h"
#include "include\helpers.h"
#include "include\shader_config.h"
#include "icc.h"
#include "include\cms_lut.h"
#include "include\info.h"
#include "include\ensure.h"

// Compiled shaders.
#include "..\ps_sample_hlsl.h"
#include "..\ps_sample_alpha_hlsl.h"
#include "..\ps_ortho_hlsl.h"
#include "..\ps_cyl_hlsl.h"
#include "..\ps_blur_hlsl.h"
#include "..\ps_sigmoidize_hlsl.h"
#include "..\ps_desigmoidize_hlsl.h"
#include "..\ps_linearize_hlsl.h"
#include "..\ps_delinearize_hlsl.h"
#include "..\ps_cms_hlsl.h"

// Use for constant buffer data.
struct Cb_data
{
    union Cb_types
    {
        float f;
        int32_t i;
        uint32_t u;
    };

    Cb_types x;
    Cb_types y;
    Cb_types z;
    Cb_types w;
};

namespace
{
    constexpr std::array WIV_PASS_FORMATS = {
        DXGI_FORMAT_R16G16B16A16_UNORM,
        DXGI_FORMAT_R32G32B32A32_FLOAT
    };

    // Max texture size will be determined by D3D_FEATURE_LEVEL_, but D3D11 and D3D12 _REQ_TEXTURE2D_U_OR_V_DIMENSION should be the same.
    template<typename T>
    constexpr T WIV_MAX_TEX_UV = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
}

void Renderer::init()
{
    create_device();
    create_swapchain();
    create_rtv_back_buffer();
    create_samplers();
    create_vertex_shader();
    if (g_config.cms_use.val) {
        init_cms_profile_display();
    }
    ui.create(device.get(), device_context.get(), &should_update);
}

void Renderer::update()
{
    if (srv_image && should_update) {
        update_scale_and_dims_output();
        update_scale_profile();
        if (!(ui.is_panning || ui.is_zooming || ui.is_rotating)) {
            srv_pass = srv_image;
            if (g_config.cms_use.val && is_cms_valid) {
                pass_cms();
            }
            if (!is_equal(scale, 1.0f)) {
                const bool sigmoidize = scale > 1.0f && p_scale_profile->sigmoid_use.val;
                bool linearize = scale < 1.0f || sigmoidize || p_scale_profile->blur_use.val;
                if (linearize) {
                    pass_linearize(image.get_width<UINT>(), image.get_height<UINT>());
                }
                if (sigmoidize) {
                    pass_sigmoidize();
                }
                if (scale < 1.0f && p_scale_profile->blur_use.val) {
                    pass_blur();
                }
                if (p_scale_profile->kernel_cylindrical_use.val) {
                    pass_cylindrical_resample();
                }
                else {
                    pass_orthogonal_resample();
                }
                if (sigmoidize) {
                    pass_desigmoidize();
                }
                if (p_scale_profile->unsharp_use.val) {
                    if (!linearize) {
                        pass_linearize(dims_output.get_width<UINT>(), dims_output.get_height<UINT>());
                        linearize = true;
                    }
                    pass_unsharp();
                }
                if (linearize) {
                    pass_delinearize(dims_output.get_width<UINT>(), dims_output.get_height<UINT>());
                }
            }
        }
        update_final_pass();
        if (ui.is_zooming) {
            should_update = true;
        }
        else {
            should_update = false;
        }
        ui.is_panning = false;
        ui.is_zooming = false;
        ui.is_rotating = false;
    }

    // Always update ui.
    ui.update();
}

void Renderer::draw() const
{
    device_context->ClearRenderTargetView(rtv_back_buffer.get(), g_config.clear_color.val.data());
    device_context->OMSetRenderTargets(1, &rtv_back_buffer, nullptr);
    device_context->Draw(3, 0);
    ui.draw();
    ensure(swapchain->Present(1, 0), >= 0);
}

// Creates the first texture from the loaded image.
void Renderer::create_image()
{
    // Get data from the image.
    DXGI_FORMAT format;
    UINT sys_mem_pitch;
    std::unique_ptr<uint8_t[]> data = get_image_data(format, sys_mem_pitch);
    
    Info::image_width = image.get_width<int>();
    Info::image_height = image.get_height<int>();

    // Create texture.
    D3D11_TEXTURE2D_DESC texture2d_desc = {};
    texture2d_desc.Width = image.get_width<UINT>();
    texture2d_desc.Height = image.get_height<UINT>();
    texture2d_desc.MipLevels = 1;
    texture2d_desc.ArraySize = 1;
    texture2d_desc.Format = format;
    texture2d_desc.SampleDesc.Count = 1;
    texture2d_desc.Usage = D3D11_USAGE_IMMUTABLE;
    texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA subresource_data = {};
    subresource_data.pSysMem = data.get();
    subresource_data.SysMemPitch = sys_mem_pitch;
    Com_ptr<ID3D11Texture2D> texture2d;
    ensure(device->CreateTexture2D(&texture2d_desc, &subresource_data, texture2d.put()), >= 0);
    ensure(device->CreateShaderResourceView(texture2d.get(), nullptr, srv_image.put()), >= 0);

    if (cms_profile_display) {
        create_cms_lut();
    }

    // We will still need a tone response curve, so use the one from the image.
    else {
        trc = image.trc;
    }
}

void Renderer::on_window_resize() noexcept
{
    // This function may get called to early, so check do we have swap chain.
    if (swapchain) {
        rtv_back_buffer.reset();
        ensure(swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0), >= 0);

        // Set swapchain dims.
        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1;
        ensure(swapchain->GetDesc1(&swap_chain_desc1), >= 0);
        dims_swap_chain.width = swap_chain_desc1.Width;
        dims_swap_chain.height = swap_chain_desc1.Height;

        create_rtv_back_buffer();
    }
}

void Renderer::reset_resources() noexcept
{
    srv_image.reset();
    create_viewport(0.0f, 0.0f);
}

// This is defined here only cause of code organization.
void Renderer::fullscreen_hide_cursor() const
{
    if (ui.is_fullscreen && !ImGui::GetIO().WantCaptureMouse && !ui.is_dialog_file_open)
        while (ShowCursor(FALSE) >= 0);
}

std::unique_ptr<uint8_t[]> Renderer::get_image_data(DXGI_FORMAT& format, UINT& sys_mem_pitch)
{
    std::unique_ptr<uint8_t[]> data;
    switch (image.get_basetype()) {
        case OIIO::TypeDesc::UINT8:
            data = image.get_image_data<uint8_t>();
            format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sys_mem_pitch = image.get_width<int>() * 4;
            break;
        case OIIO::TypeDesc::UINT16:
            data = image.get_image_data<uint16_t>();
            format = DXGI_FORMAT_R16G16B16A16_UNORM;
            sys_mem_pitch = image.get_width<int>() * 4 * 2;
            break;
        case OIIO::TypeDesc::HALF:
            data = image.get_image_data<uint16_t>();
            format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            sys_mem_pitch = image.get_width<int>() * 4 * 2;
            break;
        case OIIO::TypeDesc::FLOAT:
            data = image.get_image_data<uint32_t>();
            format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            sys_mem_pitch = image.get_width<int>() * 4 * 4;
    }
    return data;
}

void Renderer::update_scale_and_dims_output() noexcept
{
    auto image_w = image.get_width<float>();
    auto image_h = image.get_height<float>();

    // Check is the rotation angele divisible by 180, if it is we dont need to swap width and height.
    if (ui.image_rotation % 180)
        std::swap(image_w, image_h);

    float auto_zoom;
    if (ui.image_no_scale)
        auto_zoom = 0.0f;

    // Fit inside the window.
    else if (get_ratio<double>(dims_swap_chain.width, dims_swap_chain.height) > get_ratio<double>(image_w, image_h))
        auto_zoom = std::log2(get_ratio<float>(dims_swap_chain.height, image_h));
    else
        auto_zoom = std::log2(get_ratio<float>(dims_swap_chain.width, image_w));

    scale = std::pow(2.0f, auto_zoom + ui.image_zoom);

    // Limit scale so we don't exceed min or max texture dims, or stretch image.
    const auto scaled_w = image_w * scale;
    const auto scaled_h = image_h * scale;
    if (scaled_w > WIV_MAX_TEX_UV<float> || scaled_h > WIV_MAX_TEX_UV<float>)
        scale = std::min(WIV_MAX_TEX_UV<float> / image_w, WIV_MAX_TEX_UV<float> / image_h);
    else if (scaled_w < 1.0f || scaled_h < 1.0f)
        scale = std::max(1.0f / image_w, 1.0f / image_h);

    dims_output.width = static_cast<int>(std::ceil(image_w * scale));
    dims_output.height = static_cast<int>(std::ceil(image_h * scale));
    Info::scale = scale;
    Info::scaled_width = dims_output.width;
    Info::scaled_height = dims_output.height;
}

void Renderer::update_scale_profile() noexcept
{
    for (const auto& profile : g_config.scale_profiles) {
        if (profile.range.is_inrange(scale)) {
            p_scale_profile = &profile.config;
            goto info;
        }
    }

    // Else use default profile.
    p_scale_profile = &g_config.scale_profiles[0].config;

info:
    Info::kernel_index = p_scale_profile->kernel_index.val;
    Info::kernel_support = get_kernel_support();
    const auto clamped_scale = std::min(scale, 1.0f);
    Info::kernel_radius = std::ceil(Info::kernel_support / clamped_scale);
    if (p_scale_profile->kernel_cylindrical_use.val) {
        const auto a = static_cast<int>(std::ceil(Info::kernel_support / clamped_scale));
        Info::kernel_size = a * a;
    }
    else {
        Info::kernel_size = std::ceil(static_cast<float>(Info::kernel_radius) / std::min(scale, 1.0f)) * 2;
    }
    Info::scale_filter = p_scale_profile->kernel_cylindrical_use.val ? "Cylindrical" : "Orthogonal";
}

void Renderer::init_cms_profile_display()
{
    switch (g_config.cms_display_profile.val) {
        case WIV_CMS_PROFILE_DISPLAY_AUTO: {

            // Get system default ICC profile.
            auto dc = GetDC(nullptr);
            DWORD buffer_size;
            GetICMProfileA(dc, &buffer_size, nullptr);
            auto path = std::make_unique_for_overwrite<char[]>(buffer_size);
            ensure(GetICMProfileA(dc, &buffer_size, path.get()), == TRUE);
            ensure(ReleaseDC(nullptr, dc), == 1);
            
            cms_profile_display.reset(cmsOpenProfileFromFile(path.get(), "r"));
            const auto gamma = static_cast<float>(cmsDetectRGBProfileGamma(cms_profile_display.get(), 0.1));
            if (gamma > 0.0f) {
                trc = { WIV_CMS_TRC_GAMMA, gamma };
            }
            else {
                trc = { WIV_CMS_TRC_NONE, 0.0f };
            }
            break;
        }
        case WIV_CMS_PROFILE_DISPLAY_SRGB:
            cms_profile_display.reset(cmsCreate_sRGBProfile());
            trc = { WIV_CMS_TRC_SRGB, 0.0f };
            break;
        case WIV_CMS_PROFILE_DISPLAY_ADOBE:
            cms_profile_display.reset(cms_create_profile_adobe_rgb());
            trc = { WIV_CMS_TRC_GAMMA, ADOBE_RGB_GAMMA<float> };
            break;
        case WIV_CMS_PROFILE_DISPLAY_CUSTOM: {
            cms_profile_display.reset(cmsOpenProfileFromFile(g_config.cms_display_profile_custom.val.string().c_str(), "r"));
            const auto gamma = static_cast<float>(cmsDetectRGBProfileGamma(cms_profile_display.get(), 0.1));
            if (gamma > 0.0f) {
                trc = { WIV_CMS_TRC_GAMMA, gamma };
            }
            else {
                trc = { WIV_CMS_TRC_NONE, 0.0f };
            }
            break;
        }
    }
}

void Renderer::create_cms_lut()
{
    const auto lut = cms_transform_lut();
    if (!lut) {
        is_cms_valid = false;
        return;
    }

    // Bind lut as 3d texture.
    D3D11_TEXTURE3D_DESC texture3d_desc = {};
    texture3d_desc.Width = g_config.cms_lut_size.val;
    texture3d_desc.Height = g_config.cms_lut_size.val;
    texture3d_desc.Depth = g_config.cms_lut_size.val;
    texture3d_desc.MipLevels = 1;
    texture3d_desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
    texture3d_desc.Usage = D3D11_USAGE_IMMUTABLE;
    texture3d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA subresource_data = {};
    subresource_data.pSysMem = lut.get();
    subresource_data.SysMemPitch = g_config.cms_lut_size.val * 4 * 2; // width * nchannals * byte_depth
    subresource_data.SysMemSlicePitch = sq(g_config.cms_lut_size.val) * 4 * 2; // width * height * nchannals * byte_depth
    Com_ptr<ID3D11Texture3D> texture3d;
    ensure(device->CreateTexture3D(&texture3d_desc, &subresource_data, texture3d.put()), >= 0);
    Com_ptr<ID3D11ShaderResourceView> srv;
    ensure(device->CreateShaderResourceView(texture3d.get(), nullptr, srv.put()), >= 0);
    device_context->PSSetShaderResources(2, 1, &srv);
    
    is_cms_valid = true;
}

std::unique_ptr<uint16_t[]> Renderer::cms_transform_lut()
{
    std::unique_ptr<uint16_t[]> lut;
    if (image.profile) {
        
        // Set flags.
        cmsUInt32Number flags = cmsFLAGS_HIGHRESPRECALC | cmsFLAGS_NOOPTIMIZE;
        if (g_config.cms_bpc_use.val) {
            flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
        }
        
        auto htransform = cmsCreateTransform(image.profile.get(), TYPE_RGB_16, cms_profile_display.get(), TYPE_RGBA_16, g_config.cms_intent.val, flags);
        if (htransform) {
            lut = std::make_unique_for_overwrite<uint16_t[]>(cube(g_config.cms_lut_size.val) * 4);

            // Get the correct LUT.
            const void* wiv_cms_lut = nullptr;
            switch (g_config.cms_lut_size.val) {
                case 33:
                    wiv_cms_lut = WIV_CMS_LUT_33.data();
                    break;
                case 49:
                    wiv_cms_lut = WIV_CMS_LUT_49.data();
                    break;
                case 65:
                    wiv_cms_lut = WIV_CMS_LUT_65.data();
            }

            cmsDoTransform(htransform, wiv_cms_lut, lut.get(), cube(g_config.cms_lut_size.val));
            cmsDeleteTransform(htransform);
        }
    }
    return lut;
}

void Renderer::pass_cms()
{
    alignas(16) Cb_data data[1];
    data[0].x.f = g_config.cms_lut_size.val; // lut_size
    data[0].y.i = g_config.cms_dither.val && image.get_basetype() == OIIO::TypeDesc::UINT8; // dither

    // Generate a random float between 0.0 and 1.0. For dithering.
    std::srand(std::time(nullptr));
    data[0].z.f = static_cast<float>(static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX)); // random_number
    
    Com_ptr<ID3D11Buffer> cb0;
    create_constant_buffer(sizeof(data), &data, cb0.put());
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_pixel_shader(PS_CMS, sizeof(PS_CMS));
    create_viewport(image.get_width<float>(), image.get_height<float>());
    draw_pass(image.get_width<UINT>(), image.get_height<UINT>());
}

void Renderer::pass_linearize(UINT width, UINT height)
{
    if (trc.id == WIV_CMS_TRC_NONE || trc.id == WIV_CMS_TRC_LINEAR) {
        return;
    }
    alignas(16) Cb_data data[1];
    data[0].x.i = trc.id; // index

    // Only relevant if gamma correction is used.
    data[0].y.f = trc.val; // gamma_value

    Com_ptr<ID3D11Buffer> cb0;
    create_constant_buffer(sizeof(data), &data, cb0.put());
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_pixel_shader(PS_LINEARIZE, sizeof(PS_LINEARIZE));
    create_viewport(width, height);
    draw_pass(width, height);
}

void Renderer::pass_delinearize(UINT width, UINT height)
{
    if (trc.id == WIV_CMS_TRC_NONE || trc.id == WIV_CMS_TRC_LINEAR) {
        return;
    }
    alignas(16) Cb_data data[1];
    data[0].x.i = trc.id; // index

    // Only relevant if gamma correction is used.
    data[0].y.f = 1.0f / trc.val; // rcp_gamma

    Com_ptr<ID3D11Buffer> cb0;
    create_constant_buffer(sizeof(data), &data, cb0.put());
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_pixel_shader(PS_DELINEARIZE, sizeof(PS_DELINEARIZE));
    create_viewport(width, height);
    draw_pass(width, height);
}

void Renderer::pass_sigmoidize()
{
    // Sigmoidize expects linear light input.
    if (trc.id == WIV_CMS_TRC_NONE) {
        return;
    }

    // Calculate sigmoidize params.
    // pass_desigmoidize() will use these same params!
    sigmoidize_offset = 1.0f / (1.0f + std::exp(p_scale_profile->sigmoid_contrast.val * p_scale_profile->sigmoid_midpoint.val));
    sigmoidize_scale = 1.0f / (1.0f + std::exp(p_scale_profile->sigmoid_contrast.val * p_scale_profile->sigmoid_midpoint.val - p_scale_profile->sigmoid_contrast.val)) - sigmoidize_offset;

    alignas(16) Cb_data data[1];
    data[0].x.f = p_scale_profile->sigmoid_contrast.val; // contrast
    data[0].y.f = p_scale_profile->sigmoid_midpoint.val; // midpoint
    data[0].z.f = sigmoidize_offset; // offset
    data[0].w.f = sigmoidize_scale; // scale
    Com_ptr<ID3D11Buffer> cb0;
    create_constant_buffer(sizeof(data), &data, cb0.put());
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_pixel_shader(PS_SIGMOIDIZE, sizeof(PS_SIGMOIDIZE));
    create_viewport(image.get_width<float>(), image.get_height<float>());
    draw_pass(image.get_width<UINT>(), image.get_height<UINT>());
}

void Renderer::pass_desigmoidize()
{
    // Sigmoidize expects linear light input.
    if (trc.id == WIV_CMS_TRC_NONE) {
        return;
    }

    alignas(16) Cb_data data[1];
    data[0].x.f = p_scale_profile->sigmoid_contrast.val; // contrast
    data[0].y.f = p_scale_profile->sigmoid_midpoint.val; // midpoint
    data[0].z.f = sigmoidize_offset; // offset
    data[0].w.f = sigmoidize_scale; // scale
    Com_ptr<ID3D11Buffer> cb0;
    create_constant_buffer(sizeof(data), &data, cb0.put());
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_pixel_shader(PS_DESIGMOIDIZE, sizeof(PS_DESIGMOIDIZE));
    create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>());
    draw_pass(dims_output.width, dims_output.height);
}

void Renderer::pass_blur()
{
    // Pass y axis.
    //

    alignas(16) Cb_data data[2];
    data[0].x.i = p_scale_profile->blur_radius.val; // radius
    data[0].y.f = p_scale_profile->blur_sigma.val; // sigma
    data[0].z.f = 0.0f; // pt.x
    data[0].w.f = 1.0f / image.get_height<float>(); // pt.y

    // Unsharp amount, has to be <= 0!
    data[1].x.f = -1.0f; // amount

    Com_ptr<ID3D11Buffer> cb0;
    create_constant_buffer(sizeof(data), &data, cb0.put());
    create_pixel_shader(PS_BLUR, sizeof(PS_BLUR));
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_viewport(image.get_width<float>(), image.get_height<float>());
    draw_pass(image.get_width<UINT>(), image.get_height<UINT>());

    //

    // Pass x axis.
    data[0].z.f = 1.0f / image.get_width<float>(); // pt.x
    data[0].w.f = 0.0f; // pt.y
    update_constant_buffer(cb0.get(), data, sizeof(data));
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_viewport(image.get_width<float>(), image.get_height<float>());
    draw_pass(image.get_width<UINT>(), image.get_height<UINT>());
}

void Renderer::pass_unsharp()
{
    // Pass y axis.
    //

    alignas(16) Cb_data data[2];
    data[0].x.i = p_scale_profile->unsharp_radius.val; // radius
    data[0].y.f = p_scale_profile->unsharp_sigma.val; // sigma
    data[0].z.f = 0.0f; // pt.x
    data[0].w.f = 1.0f / dims_output.get_height<float>(); // pt.y

    // Unsharp amount, has to be <= 0 for the 1st pass!
    data[1].x.f = -1.0f; // amount

    Com_ptr<ID3D11Buffer> cb0;
    create_constant_buffer(sizeof(data), &data, cb0.put());
    create_pixel_shader(PS_BLUR, sizeof(PS_BLUR));
    Com_ptr<ID3D11ShaderResourceView> srv_original = srv_pass;
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>());
    draw_pass(dims_output.width, dims_output.height);

    //

    // Pass x axis.
    //
    
    data[0].z.f = 1.0f / dims_output.get_width<float>(); // pt.x
    data[0].w.f = 0.0f; // pt.y

    // Should be > 0.
    data[1].x.f = p_scale_profile->unsharp_amount.val; // amount
    
    update_constant_buffer(cb0.get(), data, sizeof(data));
    const std::array srvs = { srv_pass.get(), srv_original.get() };
    device_context->PSSetShaderResources(0, 2, srvs.data());
    create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>());
    draw_pass(dims_output.width, dims_output.height);
    
    //
}

void Renderer::pass_orthogonal_resample()
{
    // Pass y axis.
    //

    const float kernel_support = get_kernel_support();
    const float clamped_scale = std::min(scale, 1.0f);
    alignas(16) Cb_data data[4];
    data[0].x.i = p_scale_profile->kernel_index.val; // index
    data[0].y.f = kernel_support; // support
    data[0].z.f = p_scale_profile->kernel_blur.val; // blur
    data[0].w.f = p_scale_profile->kernel_parameter1.val; // p1
    data[1].x.f = p_scale_profile->kernel_parameter2.val; // p2
    
    // Antiringing shouldnt be used when downsampling!
    data[1].y.f = scale > 1.0f ? p_scale_profile->kernel_antiringing.val : -1.0f; // ar
    
    data[1].z.f = clamped_scale; // scale
    data[1].w.f = std::ceil(kernel_support / clamped_scale); // radius
    data[2].x.f = image.get_width<float>(); // dims.x
    data[2].y.f = image.get_height<float>(); // dims.y
    data[2].z.f = 1.0f / image.get_width<float>(); // pt.x
    data[2].w.f = 1.0f / image.get_height<float>(); // pt.y
    data[3].x.f = 0.0f; // axis.x
    data[3].y.f = 1.0f; // axis.y
    Com_ptr<ID3D11Buffer> cb0;
    create_constant_buffer(sizeof(data), &data, cb0.put());
    create_pixel_shader(PS_ORTHO, sizeof(PS_ORTHO));
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_viewport(image.get_width<float>(), dims_output.get_height<float>());
    draw_pass(image.get_width<UINT>(), dims_output.height);

    //

    // Pass x axis.
    data[3].x.f = 1.0f; // axis.x
    data[3].y.f = 0.0f; // axis.y	
    update_constant_buffer(cb0.get(), data, sizeof(data));
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>());
    draw_pass(dims_output.width, dims_output.height);
}

void Renderer::pass_cylindrical_resample()
{
    const float kernel_support = get_kernel_support();
    const float clamped_scale = std::min(scale, 1.0f);
    alignas(16) Cb_data data[3];
    data[0].x.i = p_scale_profile->kernel_index.val; // index
    data[0].y.f = kernel_support; // support
    data[0].z.f = p_scale_profile->kernel_blur.val; // blur
    data[0].w.f = p_scale_profile->kernel_parameter1.val; // p1
    data[1].x.f = p_scale_profile->kernel_parameter2.val; // p2

    // Antiringing shouldnt be used when downsampling!
    data[1].y.f = scale > 1.0f ? p_scale_profile->kernel_antiringing.val : -1.0f; // ar
    
    data[1].z.f = clamped_scale; // scale
    data[1].w.f = std::ceil(kernel_support / clamped_scale); // radius
    data[2].x.f = image.get_width<float>(); // dims.x
    data[2].y.f = image.get_height<float>(); // dims.y
    data[2].z.f = 1.0f / image.get_width<float>(); // pt.x
    data[2].w.f = 1.0f / image.get_height<float>(); // pt.y
    Com_ptr<ID3D11Buffer> cb0;
    create_constant_buffer(sizeof(data), &data, cb0.put());
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_pixel_shader(PS_CYL, sizeof(PS_CYL));
    create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>());
    draw_pass(dims_output.width, dims_output.height);
}

void Renderer::update_final_pass()
{
    Com_ptr<ID3D11Buffer> cb0;
    if (image.has_alpha()) {
        alignas(16) Cb_data data[3];
        data[0].x.f = dims_output.get_width<float>() / g_config.alpha_tile_size.val; // size.x
        data[0].y.f = dims_output.get_height<float>() / g_config.alpha_tile_size.val; // size.y
        data[0].z.f = ui.image_rotation; // theta

        // Check is theta divisible by 360, if it is we dont need to rotate texcoord.
        data[0].w.i = ui.image_rotation % 360; // rotate

        data[1].x.f = g_config.alpha_tile1_color.val[0]; // tile1.x
        data[1].y.f = g_config.alpha_tile1_color.val[1]; // tile1.y
        data[1].z.f = g_config.alpha_tile1_color.val[2]; // tile1.z
        data[2].x.f = g_config.alpha_tile2_color.val[0]; // tile2.x
        data[2].y.f = g_config.alpha_tile2_color.val[1]; // tile2.y
        data[2].z.f = g_config.alpha_tile2_color.val[2]; // tile2.z
        create_constant_buffer(sizeof(data), &data, cb0.put());
        create_pixel_shader(PS_SAMPLE_ALPHA, sizeof(PS_SAMPLE_ALPHA));
    }
    else {
        alignas(16) Cb_data data[1];
        data[0].x.f = ui.image_rotation; // theta

        // Check is theta divisible by 360, if it is we dont need to rotate texcoord.
        data[0].y.i = ui.image_rotation % 360; // rotate

        create_constant_buffer(sizeof(data), &data, cb0.put());
        create_pixel_shader(PS_SAMPLE, sizeof(PS_SAMPLE));
    }
    device_context->PSSetShaderResources(0, 1, &srv_pass);
    create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>(), true);
}

void Renderer::draw_pass(UINT width, UINT height) noexcept
{
    // Create texture.
    D3D11_TEXTURE2D_DESC texture2d_desc = {};
    texture2d_desc.Width = width;
    texture2d_desc.Height = height;
    texture2d_desc.MipLevels = 1;
    texture2d_desc.ArraySize = 1;
    texture2d_desc.Format = WIV_PASS_FORMATS[g_config.pass_format.val];
    texture2d_desc.SampleDesc.Count = 1;
    texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    Com_ptr<ID3D11Texture2D> texture2d;
    ensure(device->CreateTexture2D(&texture2d_desc, nullptr, texture2d.put()), >= 0);
    
    // Create render target view.
    Com_ptr<ID3D11RenderTargetView> rtv;
    ensure(device->CreateRenderTargetView(texture2d.get(), nullptr, rtv.put()), >= 0);

    // Draw to the render target view.
    device_context->OMSetRenderTargets(1, &rtv, nullptr);
    device_context->Draw(3, 0);
    unbind_render_targets();

    ensure(device->CreateShaderResourceView(texture2d.get(), nullptr, srv_pass.put()), >= 0);
}

void Renderer::create_viewport(float width, float height, bool adjust) const noexcept
{
    D3D11_VIEWPORT viewport = {};
    viewport.Width = width;
    viewport.Height = height;

    // Offset image in order to center it in the window + apply panning.
    if (adjust) {
        viewport.TopLeftX = (dims_swap_chain.width - viewport.Width) / 2.0f + ui.image_pan.x;
        viewport.TopLeftY = (dims_swap_chain.height - viewport.Height) / 2.0f + ui.image_pan.y;
    }
    
    device_context->RSSetViewports(1, &viewport);
}

float Renderer::get_kernel_support() const noexcept
{
    switch (p_scale_profile->kernel_index.val) {
        case WIV_KERNEL_FUNCTION_NEAREST:
            if (p_scale_profile->kernel_cylindrical_use.val) {
                return std::numbers::sqrt2_v<float> / 2.0f;
            }
            return 1.0f / 2.0f;
        case WIV_KERNEL_FUNCTION_LINEAR:
            if (p_scale_profile->kernel_cylindrical_use.val) {
                return std::numbers::sqrt2_v<float>;
            }
            return 1.0f;
        case WIV_KERNEL_FUNCTION_BICUBIC:
        case WIV_KERNEL_FUNCTION_BCSPLINE:
            return 2.0f;
        case WIV_KERNEL_FUNCTION_FSR:
            if (p_scale_profile->kernel_cylindrical_use.val) {
                return 2.233131f; // Second Jinc zero.
            }
            return 2.0f;
        default:
            return p_scale_profile->kernel_support.val;
    }
}

#pragma once

#include "pch.h"
#include "config.h"
#include "user_interface.h"
#include "include\dims.h"
#include "renderer_base.h"
#include "include\shader_config.h"

enum WIV_CMS_PROFILE_DISPLAY_
{
    WIV_CMS_PROFILE_DISPLAY_AUTO,
    WIV_CMS_PROFILE_DISPLAY_SRGB,
    WIV_CMS_PROFILE_DISPLAY_ADOBE,
    WIV_CMS_PROFILE_DISPLAY_CUSTOM
};

class Renderer : Renderer_base
{
public:
    void init();
    void update();
    void draw() const;
    void create_image();
    void on_window_resize() noexcept;
    void reset_resources() noexcept;
    void fullscreen_hide_cursor() const;
    bool should_update;
    User_interface ui;
private:
    std::unique_ptr<uint8_t[]> get_image_data(DXGI_FORMAT& format, UINT& sys_mem_pitch);
    void update_scale_and_dims_output() noexcept;
    void update_scale_profile() noexcept;
    void init_cms_profile_display();
    void create_cms_lut();
    std::unique_ptr<uint16_t[]> cms_transform_lut();
    void pass_cms();
    void pass_linearize(UINT width, UINT height);
    void pass_delinearize(UINT width, UINT height);
    void pass_sigmoidize();
    void pass_desigmoidize();
    void pass_blur();
    void pass_unsharp();
    void pass_orthogonal_resample();
    void pass_cylindrical_resample();
    void update_final_pass();
    void draw_pass(UINT width, UINT height) noexcept;
    void create_viewport(float width, float height, bool adjust = false) const noexcept;
    float get_kernel_support() const noexcept;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_image;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_pass;
    Image& image = ui.file_manager.image;
    Dims<int> dims_output;
    float scale;
    const Config_scale* p_scale_profile;
    std::unique_ptr<std::remove_pointer_t<cmsHPROFILE>, decltype(&cmsCloseProfile)> cms_profile_display = { nullptr, cmsCloseProfile };
    Tone_response_curve trc;
    bool is_cms_valid;
    float sigmoidize_offset;
    float sigmoidize_scale;
};

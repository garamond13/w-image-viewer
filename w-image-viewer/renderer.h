#pragma once

#include "pch.h"
#include "config.h"
#include "user_interface.h"
#include "dims.h"

//note that in hlsl sizeof bool is 4 bytes
union Cb_types
{
	float f;
	int32_t i;
	uint32_t ui;
};

//use for constant buffer data
struct alignas(16) Cb4
{
	Cb_types x;
	Cb_types y;
	Cb_types z;
	Cb_types w;
};

class Renderer
{
public:
	void create(Config* config, HWND hwnd);
	void update();
	void draw() const;
	void create_image();
	void on_window_resize() noexcept;
	void reset_resources() noexcept;
	void fullscreen_hide_cursor() const;
	bool should_update;
	User_interface user_interface;
private:
	void create_device();
	void create_swapchain(HWND hwnd);
	void create_rtv_back_buffer() noexcept;
	void create_samplers() const;
	void create_vertex_shader() const noexcept;
	void create_cms_profile_display();
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
	void pass_last();
	void draw_pass(UINT width, UINT height) noexcept;
	void create_constant_buffer(ID3D11Buffer** buffer, UINT byte_width) const noexcept;
	void update_constant_buffer(ID3D11Buffer* buffer, const void* data, size_t size) const noexcept;
	void create_pixel_shader(const BYTE* shader, size_t shader_size) const noexcept;
	void create_viewport(float width, float height, bool adjust = false) const noexcept;
	void update_dims_output();
	void update_scale() noexcept;
	float get_kernel_radius() const noexcept;
	void update_trc();
	Config* p_config;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_back_buffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_image;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_pass;
	Image& image{ user_interface.file_manager.image };
	Dims<int> dims_swap_chain;
	Dims<int> dims_output;
	float scale;
	std::unique_ptr<std::remove_pointer<cmsHPROFILE>::type, decltype(&cmsCloseProfile)> cms_profile_display{ nullptr, cmsCloseProfile };
	std::pair<int, float> trc; //tone responce curve, type and value
	bool is_cms_valid;
};

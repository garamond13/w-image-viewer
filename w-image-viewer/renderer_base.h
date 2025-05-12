#pragma once

#include "pch.h"
#include "dims.h"

class Renderer_base
{
protected:
	void create_device() noexcept;
	void create_swapchain();
	void create_rtv_back_buffer() noexcept;
	void create_samplers() const;
	void create_vertex_shader() const noexcept;
	void create_pixel_shader(const BYTE* shader, size_t shader_size) const noexcept;
	void create_constant_buffer(UINT byte_width, const void* data, ID3D11Buffer** buffer) const noexcept;
	void update_constant_buffer(ID3D11Buffer* buffer, const void* data, size_t size) const noexcept;
	void unbind_render_targets() const;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_back_buffer;
	Dims dims_swap_chain;
};
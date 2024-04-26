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
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_back_buffer;
	Dims dims_swap_chain;
};
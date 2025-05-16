#include "pch.h"
#include "renderer_base.h"
#include "helpers.h"
#include "global.h"
#include "ensure.h"

// Compiled shaders.
#include "vs_quad_hlsl.h"

void Renderer_base::create_device() noexcept
{
#ifdef NDEBUG
	constexpr UINT flags = 0;
#else
	constexpr UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	static constinit const std::array feature_levels = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};
	ensure(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, feature_levels.data(), feature_levels.size(), D3D11_SDK_VERSION, device.ReleaseAndGetAddressOf(), nullptr, device_context.ReleaseAndGetAddressOf()), == S_OK);
}

void Renderer_base::create_swapchain()
{
	// Query interfaces.
	Microsoft::WRL::ComPtr<IDXGIDevice1> dxgi_device1;
	ensure(device.As(&dxgi_device1), == S_OK);
	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;
	ensure(dxgi_device1->GetAdapter(dxgi_adapter.GetAddressOf()), == S_OK);
	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory2;
	ensure(dxgi_adapter->GetParent(IID_PPV_ARGS(dxgi_factory2.GetAddressOf())), == S_OK);

	// Create swap chain.
	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1 = {
		.Format = DXGI_FORMAT_R10G10B10A2_UNORM,
		.SampleDesc = { .Count = 1 },
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = 2,
		.Scaling = DXGI_SCALING_NONE,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD
	};
	ensure(dxgi_factory2->CreateSwapChainForHwnd(dxgi_device1.Get(), g_hwnd, &swap_chain_desc1, nullptr, nullptr, swapchain.ReleaseAndGetAddressOf()), == S_OK);

	// Set member swapchain dims.
	ensure(swapchain->GetDesc1(&swap_chain_desc1), == S_OK);
	dims_swap_chain.width = swap_chain_desc1.Width;
	dims_swap_chain.height = swap_chain_desc1.Height;

	ensure(dxgi_device1->SetMaximumFrameLatency(1), == S_OK);

	// Disable exclusive fullscreen.
	ensure(dxgi_factory2->MakeWindowAssociation(g_hwnd, DXGI_MWA_NO_ALT_ENTER), == S_OK);
}

void Renderer_base::create_rtv_back_buffer() noexcept
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
	ensure(swapchain->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf())), == S_OK);
	ensure(device->CreateRenderTargetView(back_buffer.Get(), nullptr, rtv_back_buffer.ReleaseAndGetAddressOf()), == S_OK);
}

void Renderer_base::create_samplers() const
{
	// Create point sampler.
	D3D11_SAMPLER_DESC sampler_desc = {
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_NEVER,
	};
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_point;
	ensure(device->CreateSamplerState(&sampler_desc, sampler_state_point.GetAddressOf()), == S_OK);

	// Create linear sampler.
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_linear;
	ensure(device->CreateSamplerState(&sampler_desc, sampler_state_linear.GetAddressOf()), == S_OK);

	std::array sampler_states = { sampler_state_point.Get(), sampler_state_linear.Get() };
	device_context->PSSetSamplers(0, sampler_states.size(), sampler_states.data());
}

void Renderer_base::create_vertex_shader() const noexcept
{
	device_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	ensure(device->CreateVertexShader(VS_QUAD, sizeof(VS_QUAD), nullptr, vertex_shader.GetAddressOf()), == S_OK);
	device_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
}

void Renderer_base::create_pixel_shader(const BYTE* shader, size_t shader_size) const noexcept
{
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	ensure(device->CreatePixelShader(shader, shader_size, nullptr, pixel_shader.GetAddressOf()), == S_OK);
	device_context->PSSetShader(pixel_shader.Get(), nullptr, 0);
}

void Renderer_base::create_constant_buffer(UINT byte_width, const void* data, ID3D11Buffer** buffer) const noexcept
{
	const D3D11_BUFFER_DESC buffer_desc = {
		.ByteWidth = byte_width,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};
	const D3D11_SUBRESOURCE_DATA subresource_data = {
		.pSysMem = data
	};
	ensure(device->CreateBuffer(&buffer_desc, &subresource_data, buffer), == S_OK);
	device_context->PSSetConstantBuffers(0, 1, buffer);
}

void Renderer_base::update_constant_buffer(ID3D11Buffer* buffer, const void* data, size_t size) const noexcept
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource;
	ensure(device_context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource), == S_OK);
	std::memcpy(mapped_subresource.pData, data, size);
	device_context->Unmap(buffer, 0);
}

void Renderer_base::unbind_render_targets() const
{
	static constinit ID3D11RenderTargetView* rtvs[] = { nullptr };
	device_context->OMSetRenderTargets(1, rtvs, nullptr);
}

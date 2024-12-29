#include "pch.h"
#include "renderer_base.h"
#include "helpers.h"
#include "global.h"

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
	wiv_assert(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, feature_levels.data(), feature_levels.size(), D3D11_SDK_VERSION, device.ReleaseAndGetAddressOf(), nullptr, device_context.ReleaseAndGetAddressOf()), == S_OK);
}

void Renderer_base::create_swapchain()
{
	// Query interfaces.
	Microsoft::WRL::ComPtr<IDXGIDevice1> dxgi_device2;
	wiv_assert(device->QueryInterface(IID_PPV_ARGS(dxgi_device2.ReleaseAndGetAddressOf())), == S_OK);
	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;
	wiv_assert(dxgi_device2->GetAdapter(dxgi_adapter.ReleaseAndGetAddressOf()), == S_OK);
	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory2;
	wiv_assert(dxgi_adapter->GetParent(IID_PPV_ARGS(dxgi_factory2.ReleaseAndGetAddressOf())), == S_OK);

	// Create swap chain.
	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1 = {
		.Format = DXGI_FORMAT_R10G10B10A2_UNORM,
		.SampleDesc = { .Count = 1 },
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = 2,
		.Scaling = DXGI_SCALING_NONE,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD
	};
	wiv_assert(dxgi_factory2->CreateSwapChainForHwnd(dxgi_device2.Get(), g_hwnd, &swap_chain_desc1, nullptr, nullptr, swap_chain.ReleaseAndGetAddressOf()), == S_OK);

	// Set member swapchain dims.
	wiv_assert(swap_chain->GetDesc1(&swap_chain_desc1), == S_OK);
	dims_swap_chain.width = swap_chain_desc1.Width;
	dims_swap_chain.height = swap_chain_desc1.Height;

	wiv_assert(dxgi_device2->SetMaximumFrameLatency(1), == S_OK);

	// Disable exclusive fullscreen.
	wiv_assert(dxgi_factory2->MakeWindowAssociation(g_hwnd, DXGI_MWA_NO_ALT_ENTER), == S_OK);
}

void Renderer_base::create_rtv_back_buffer() noexcept
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
	wiv_assert(swap_chain->GetBuffer(0, IID_PPV_ARGS(back_buffer.ReleaseAndGetAddressOf())), == S_OK);
	wiv_assert(device->CreateRenderTargetView(back_buffer.Get(), nullptr, rtv_back_buffer.ReleaseAndGetAddressOf()), == S_OK);
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
	wiv_assert(device->CreateSamplerState(&sampler_desc, sampler_state_point.ReleaseAndGetAddressOf()), == S_OK);

	// Create linear sampler.
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_linear;
	wiv_assert(device->CreateSamplerState(&sampler_desc, sampler_state_linear.ReleaseAndGetAddressOf()), == S_OK);

	std::array sampler_states = { sampler_state_point.Get(), sampler_state_linear.Get() };
	device_context->PSSetSamplers(0, sampler_states.size(), sampler_states.data());
}

void Renderer_base::create_vertex_shader() const noexcept
{
	device_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	wiv_assert(device->CreateVertexShader(VS_QUAD, sizeof(VS_QUAD), nullptr, vertex_shader.ReleaseAndGetAddressOf()), == S_OK);
	device_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
}

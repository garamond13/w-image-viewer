#include "pch.h"
#include "renderer_base.h"
#include "include\helpers.h"
#include "include\global.h"
#include "include\ensure.h"

// Compiled shaders.
#include "..\vs_quad_hlsl.h"

void Renderer_base::create_device() noexcept
{
#ifdef NDEBUG
    constexpr UINT flags = 0;
#else
    constexpr UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#endif

    static constexpr std::array feature_levels = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };
    ensure(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, feature_levels.data(), feature_levels.size(), D3D11_SDK_VERSION, device.put(), nullptr, device_context.put()), >= 0);
}

void Renderer_base::create_swapchain()
{
    // Query interfaces.
    Com_ptr<IDXGIDevice1> dxgi_device1;
    ensure(device->QueryInterface(dxgi_device1.put()), >= 0);
    Com_ptr<IDXGIAdapter> dxgi_adapter;
    ensure(dxgi_device1->GetAdapter(dxgi_adapter.put()), >= 0);
    Com_ptr<IDXGIFactory2> dxgi_factory2;
    ensure(dxgi_adapter->GetParent(IID_PPV_ARGS(dxgi_factory2.put())), >= 0);

    // Create swap chain.
    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1 = {};
    swap_chain_desc1.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
    swap_chain_desc1.SampleDesc.Count = 1;
    swap_chain_desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc1.BufferCount = 2;
    swap_chain_desc1.Scaling = DXGI_SCALING_NONE;
    swap_chain_desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    ensure(dxgi_factory2->CreateSwapChainForHwnd(dxgi_device1.get(), g_hwnd, &swap_chain_desc1, nullptr, nullptr, swapchain.put()), >= 0);

    // Set member swapchain dims.
    ensure(swapchain->GetDesc1(&swap_chain_desc1), >= 0);
    dims_swap_chain.width = swap_chain_desc1.Width;
    dims_swap_chain.height = swap_chain_desc1.Height;

    ensure(dxgi_device1->SetMaximumFrameLatency(1), >= 0);

    // Disable exclusive fullscreen.
    ensure(dxgi_factory2->MakeWindowAssociation(g_hwnd, DXGI_MWA_NO_ALT_ENTER), >= 0);
}

void Renderer_base::create_rtv_back_buffer() noexcept
{
    Com_ptr<ID3D11Texture2D> back_buffer;
    ensure(swapchain->GetBuffer(0, IID_PPV_ARGS(back_buffer.put())), >= 0);
    ensure(device->CreateRenderTargetView(back_buffer.get(), nullptr, rtv_back_buffer.put()), >= 0);
}

void Renderer_base::create_samplers() const
{
    // Create point sampler.
    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.MaxAnisotropy = 1;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    Com_ptr<ID3D11SamplerState> sampler_state_point;
    ensure(device->CreateSamplerState(&sampler_desc, sampler_state_point.put()), >= 0);

    // Create linear sampler.
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    Com_ptr<ID3D11SamplerState> sampler_state_linear;
    ensure(device->CreateSamplerState(&sampler_desc, sampler_state_linear.put()), >= 0);

    const std::array sampler_states = { sampler_state_point.get(), sampler_state_linear.get() };
    device_context->PSSetSamplers(0, sampler_states.size(), sampler_states.data());
}

void Renderer_base::create_vertex_shader() const noexcept
{
    device_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Com_ptr<ID3D11VertexShader> vertex_shader;
    ensure(device->CreateVertexShader(VS_QUAD, sizeof(VS_QUAD), nullptr, vertex_shader.put()), >= 0);
    device_context->VSSetShader(vertex_shader.get(), nullptr, 0);
}

void Renderer_base::create_pixel_shader(const BYTE* shader, size_t shader_size) const noexcept
{
    Com_ptr<ID3D11PixelShader> pixel_shader;
    ensure(device->CreatePixelShader(shader, shader_size, nullptr, pixel_shader.put()), >= 0);
    device_context->PSSetShader(pixel_shader.get(), nullptr, 0);
}

void Renderer_base::create_constant_buffer(UINT byte_width, const void* data, ID3D11Buffer** buffer) const noexcept
{
    D3D11_BUFFER_DESC buffer_desc = {};
    buffer_desc.ByteWidth = byte_width;
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA subresource_data = {};
    subresource_data.pSysMem = data;
    ensure(device->CreateBuffer(&buffer_desc, &subresource_data, buffer), >= 0);
    device_context->PSSetConstantBuffers(0, 1, buffer);
}

void Renderer_base::update_constant_buffer(ID3D11Buffer* buffer, const void* data, size_t size) const noexcept
{
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;
    ensure(device_context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource), >= 0);
    std::memcpy(mapped_subresource.pData, data, size);
    device_context->Unmap(buffer, 0);
}

void Renderer_base::unbind_render_targets() const
{
    static constinit ID3D11RenderTargetView* rtvs[] = { nullptr };
    device_context->OMSetRenderTargets(1, rtvs, nullptr);
}

#include "pch.h"
#include "renderer.h"
#include "helpers.h"
#include "shader_config.h"
#include "icc.h"
#include "cms_lut.h"

//compiled shaders
#include "vs_quad_hlsl.h"
#include "ps_sample_hlsl.h"
#include "ps_sample_alpha_hlsl.h"
#include "ps_ortho_hlsl.h"
#include "ps_cyl_hlsl.h"
#include "ps_blur_hlsl.h"
#include "ps_sigmoidize_hlsl.h"
#include "ps_desigmoidize_hlsl.h"
#include "ps_linearize_hlsl.h"
#include "ps_delinearize_hlsl.h"
#include "ps_cms_hlsl.h"

void Renderer::create(Config* config, HWND hwnd)
{
	this->p_config = config;
	create_device();
	create_swapchain(hwnd);
	create_samplers();
	create_vertex_shader();
	create_cms_profile_display();
	user_interface.create(config, hwnd, device.Get(), device_context.Get());
}

void Renderer::update()
{
	if (srv_image && should_update) {
		srv_pass = srv_image;
		update_dims_output();
		update_scale();
		if (p_config->cms_use && is_cms_valid)
			pass_cms();
		if (!is_equal(scale, 1.0f)) {
			update_tone_responce_curve();
			bool sigmoidize{ scale > 1.0f && p_config->sigmoid_use };
			bool linearize{ scale < 1.0f || sigmoidize || p_config->blur_use };
			if (linearize)
				pass_linearize(image.get_width<UINT>(), image.get_height<UINT>());
			if (sigmoidize)
				pass_sigmoidize();
			if (scale < 1.0f && p_config->blur_use)
				pass_blur();
			if (p_config->kernel_use_cyl)
				pass_cylindrical_resample();
			else
				pass_orthogonal_resample();
			if (sigmoidize)
				pass_desigmoidize();
			if (p_config->unsharp_use) {
				if (!linearize) {
					pass_linearize(dims_output.width, dims_output.height);
					linearize = true;
				}
				pass_unsharp();
			}
			if (linearize)
				pass_delinearize(dims_output.width, dims_output.height);
		}
		pass_last();
		should_update = false;
	}
	user_interface.update();
}

void Renderer::draw() const
{
	device_context->OMSetRenderTargets(1, rtv_back_buffer.GetAddressOf(), nullptr);
	device_context->ClearRenderTargetView(rtv_back_buffer.Get(), p_config->clear_c.data());
	device_context->Draw(3, 0);
	user_interface.draw();
	wiv_assert(swap_chain->Present(1, 0), == S_OK);
}

void Renderer::create_image()
{
	//get data from image
	DXGI_FORMAT format;
	UINT sys_mem_pitch;
	std::unique_ptr<uint8_t[]> data{ image.get_data(format, sys_mem_pitch) };

	//create texture
	const D3D11_TEXTURE2D_DESC texture2d_desc{
		.Width{ image.get_width<UINT>() },
		.Height{ image.get_height<UINT>() },
		.MipLevels{ 1 },
		.ArraySize{ 1 },
		.Format{ format },
		.SampleDesc{
			.Count{ 1 },
		},
		.Usage{ D3D11_USAGE_IMMUTABLE },
		.BindFlags{ D3D11_BIND_SHADER_RESOURCE }
	};
	const D3D11_SUBRESOURCE_DATA subresource_data{
		.pSysMem{ data.get() },
		.SysMemPitch{ sys_mem_pitch }
	};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
	wiv_assert(device->CreateTexture2D(&texture2d_desc, &subresource_data, texture2d.ReleaseAndGetAddressOf()), == S_OK);
	wiv_assert(device->CreateShaderResourceView(texture2d.Get(), nullptr, srv_image.ReleaseAndGetAddressOf()), == S_OK);

	if (p_config->cms_use && cms_profile_display)
		create_cms_lut();
}

void Renderer::on_window_resize() noexcept
{
	if (swap_chain) {
		rtv_back_buffer.Reset();
		wiv_assert(swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0), == S_OK);

		//set swapchain dims
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1;
		wiv_assert(swap_chain->GetDesc1(&swap_chain_desc1), == S_OK);
		dims_swap_chain.width = swap_chain_desc1.Width;
		dims_swap_chain.height = swap_chain_desc1.Height;

		create_rtv_back_buffer();
	}
}

void Renderer::reset_resources() noexcept
{
	srv_image.Reset();
	create_viewport(0.0f, 0.0f);
}

void Renderer::fullscreen_hide_cursor() const
{
	if (user_interface.is_fullscreen && !ImGui::GetIO().WantCaptureMouse && !user_interface.is_dialog_file_open)
		while (ShowCursor(FALSE) >= 0);
}

void Renderer::create_device()
{
#ifdef NDEBUG
	constexpr UINT flags{};
#else
	constexpr UINT flags{ D3D11_CREATE_DEVICE_DEBUG };
#endif

	constexpr std::array feature_levels{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};
	wiv_assert(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, feature_levels.data(), feature_levels.size(), D3D11_SDK_VERSION, device.ReleaseAndGetAddressOf(), nullptr, device_context.ReleaseAndGetAddressOf()), == S_OK);
}

void Renderer::create_swapchain(HWND hwnd)
{
	//query interfaces
	Microsoft::WRL::ComPtr<IDXGIDevice1> dxgi_device2;
	wiv_assert(device->QueryInterface(IID_PPV_ARGS(dxgi_device2.ReleaseAndGetAddressOf())), == S_OK);
	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;
	wiv_assert(dxgi_device2->GetAdapter(dxgi_adapter.ReleaseAndGetAddressOf()), == S_OK);
	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory2;
	wiv_assert(dxgi_adapter->GetParent(IID_PPV_ARGS(dxgi_factory2.ReleaseAndGetAddressOf())), == S_OK);

	//create swap chain
	//

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1{
		.SampleDesc{
			.Count{ 1 },
		},
		.BufferUsage{ DXGI_USAGE_RENDER_TARGET_OUTPUT },
		.BufferCount{ 2 },
		.Scaling{ DXGI_SCALING_NONE },
		.SwapEffect{ DXGI_SWAP_EFFECT_FLIP_DISCARD }
	};
	
	//get primary output
	Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
	wiv_assert(dxgi_adapter->EnumOutputs(0, dxgi_output.ReleaseAndGetAddressOf()), == S_OK);

	//get display color bit depth
	Microsoft::WRL::ComPtr<IDXGIOutput6> dxgi_output6;
	wiv_assert(dxgi_output->QueryInterface(IID_PPV_ARGS(dxgi_output6.ReleaseAndGetAddressOf())), == S_OK);
	DXGI_OUTPUT_DESC1 output_desc1;
	wiv_assert(dxgi_output6->GetDesc1(&output_desc1), == S_OK);
	
	swap_chain_desc1.Format = output_desc1.BitsPerColor == 10 ? DXGI_FORMAT_R10G10B10A2_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
	wiv_assert(dxgi_factory2->CreateSwapChainForHwnd(dxgi_device2.Get(), hwnd, &swap_chain_desc1, nullptr, nullptr, swap_chain.ReleaseAndGetAddressOf()), == S_OK);

	//

	//set swapchain dims
	wiv_assert(swap_chain->GetDesc1(&swap_chain_desc1), == S_OK);
	dims_swap_chain.width = swap_chain_desc1.Width;
	dims_swap_chain.height = swap_chain_desc1.Height;

	//disable exclusive fullscreen
	wiv_assert(dxgi_factory2->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER), == S_OK);

	create_rtv_back_buffer();
}

void Renderer::create_rtv_back_buffer() noexcept
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
	wiv_assert(swap_chain->GetBuffer(0, IID_PPV_ARGS(back_buffer.ReleaseAndGetAddressOf())), == S_OK);
	wiv_assert(device->CreateRenderTargetView(back_buffer.Get(), nullptr, rtv_back_buffer.ReleaseAndGetAddressOf()), == S_OK);
}

void Renderer::create_samplers() const
{
	//create point sampler
	D3D11_SAMPLER_DESC sampler_desc{
		.AddressU{ D3D11_TEXTURE_ADDRESS_CLAMP },
		.AddressV{ D3D11_TEXTURE_ADDRESS_CLAMP },
		.AddressW{ D3D11_TEXTURE_ADDRESS_CLAMP },
		.MaxAnisotropy{ 1 },
		.ComparisonFunc{ D3D11_COMPARISON_NEVER },
		.MaxLOD{ D3D11_FLOAT32_MAX }
	};
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_point;
	wiv_assert(device->CreateSamplerState(&sampler_desc, sampler_state_point.ReleaseAndGetAddressOf()), == S_OK);

	//create linear sampler
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_linear;
	wiv_assert(device->CreateSamplerState(&sampler_desc, sampler_state_linear.ReleaseAndGetAddressOf()), == S_OK);

	std::array sampler_states{ sampler_state_point.Get(), sampler_state_linear.Get() };
	device_context->PSSetSamplers(0, sampler_states.size(), sampler_states.data());
}

void Renderer::create_vertex_shader() const noexcept
{
	device_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	wiv_assert(device->CreateVertexShader(VS_QUAD, sizeof(VS_QUAD), nullptr, vertex_shader.ReleaseAndGetAddressOf()), == S_OK);
	device_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
}

void Renderer::create_cms_profile_display()
{
	switch (p_config->cms_profile_display) {
	case WIV_CMS_PROFILE_DISPLAY_AUTO: {

		//get system default icc profile
		auto dc{ GetDC(nullptr) };
		DWORD size;
		GetICMProfileA(dc, &size, nullptr);
		auto path{ std::make_unique_for_overwrite<char[]>(size) };
		wiv_assert(GetICMProfileA(dc, &size, path.get()), == TRUE);
		wiv_assert(ReleaseDC(nullptr, dc), == 1);
		
		cms_profile_display.reset(cmsOpenProfileFromFile(path.get(), "r"));
		break;
	}
	case WIV_CMS_PROFILE_DISPLAY_SRGB:
		cms_profile_display.reset(cmsCreate_sRGBProfile());
		break;
	case WIV_CMS_PROFILE_DISPLAY_ADOBE:
		cms_profile_display.reset(cms_create_profile_adobe_rgb());
		break;
	case WIV_CMS_PROFILE_DISPLAY_CUSTOM:
		cms_profile_display.reset(cmsOpenProfileFromFile(p_config->cms_profile_display_custom.string().c_str(), "r"));
		break;
	}
}

void Renderer::create_cms_lut()
{
	auto lut{ cms_transform_lut() };
	if (!lut) {
		is_cms_valid = false;
		return;
	}

	//bind lut as 3d texture
	constexpr D3D11_TEXTURE3D_DESC texture3d_desc{
		.Width{ WIV_CMS_LUT_SIZE },
		.Height{ WIV_CMS_LUT_SIZE },
		.Depth{ WIV_CMS_LUT_SIZE },
		.MipLevels{ 1 },
		.Format{ DXGI_FORMAT_R16G16B16A16_UNORM },
		.Usage{ D3D11_USAGE_IMMUTABLE },
		.BindFlags{ D3D11_BIND_SHADER_RESOURCE },
	};
	const D3D11_SUBRESOURCE_DATA subresource_data{
		.pSysMem{ lut.get() },
		.SysMemPitch{ WIV_CMS_LUT_SIZE * 4 * 2 }, //width * nchannals * bytedepth
		.SysMemSlicePitch{ WIV_CMS_LUT_SIZE * WIV_CMS_LUT_SIZE * 4 * 2 }, //width * height * nchannals * bytedepth
	};
	Microsoft::WRL::ComPtr<ID3D11Texture3D> texture3d;
	wiv_assert(device->CreateTexture3D(&texture3d_desc, &subresource_data, texture3d.ReleaseAndGetAddressOf()), == S_OK);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	wiv_assert(device->CreateShaderResourceView(texture3d.Get(), nullptr, srv.ReleaseAndGetAddressOf()), == S_OK);
	device_context->PSSetShaderResources(2, 1, srv.GetAddressOf());
	
	is_cms_valid = true;
}

std::unique_ptr<uint16_t[]> Renderer::cms_transform_lut()
{
	std::unique_ptr<uint16_t[]> lut;
	cmsUInt32Number flags{ cmsFLAGS_NOCACHE | cmsFLAGS_HIGHRESPRECALC | cmsFLAGS_NOOPTIMIZE };
	if (p_config->cms_use_bpc)
		flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
	auto htransform{ cmsCreateTransform(image.embended_profile.get(), TYPE_RGBA_16, cms_profile_display.get(), TYPE_RGBA_16, p_config->cms_intent, flags)};
	if (htransform) {
		lut = std::make_unique_for_overwrite<uint16_t[]>(WIV_CMS_LUT_SIZE * WIV_CMS_LUT_SIZE * WIV_CMS_LUT_SIZE * 4);
		cmsDoTransform(htransform, WIV_CMS_LUT, lut.get(), WIV_CMS_LUT_SIZE * WIV_CMS_LUT_SIZE * WIV_CMS_LUT_SIZE);
		cmsDeleteTransform(htransform);
	}
	return lut;
}

void Renderer::pass_cms()
{
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_pixel_shader(PS_CMS, sizeof(PS_CMS));
	create_viewport(image.get_width<float>(), image.get_height<float>());
	draw_pass(image.get_width<UINT>(), image.get_height<UINT>());
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);
}

void Renderer::pass_linearize(UINT width, UINT height)
{
	const alignas(16) std::array data{
		Cb4{
			.x{ .i{ trc.first }},
			.y{ .f{ trc.second }} //gamma, only relevant if gamma correction is used
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
	create_constant_buffer(cb0.ReleaseAndGetAddressOf(), sizeof(data));
	update_constant_buffer(cb0.Get(), data.data(), sizeof(data));
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_pixel_shader(PS_LINEARIZE, sizeof(PS_LINEARIZE));
	create_viewport(static_cast<float>(width), static_cast<float>(height));
	draw_pass(width, height);
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);
}

void Renderer::pass_delinearize(UINT width, UINT height)
{
	const alignas(16) std::array data{
		Cb4{
			.x{ .i{ trc.first }},
			.y{ .f{ 1.0f / trc.second }} //1.0 / gamma, only relevant if gamma correction is used
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
	create_constant_buffer(cb0.ReleaseAndGetAddressOf(), sizeof(data));
	update_constant_buffer(cb0.Get(), data.data(), sizeof(data));
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_pixel_shader(PS_DELINEARIZE, sizeof(PS_DELINEARIZE));
	create_viewport(static_cast<float>(width), static_cast<float>(height));
	draw_pass(width, height);
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);
}

void Renderer::pass_sigmoidize()
{
	const alignas(16) std::array data{
		Cb4{
			.x{ .f{ p_config->sigmoid_c }},
			.y{ .f{ p_config->sigmoid_m }}
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
	create_constant_buffer(cb0.ReleaseAndGetAddressOf(), sizeof(data));
	update_constant_buffer(cb0.Get(), data.data(), sizeof(data));
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_pixel_shader(PS_SIGMOIDIZE, sizeof(PS_SIGMOIDIZE));
	create_viewport(image.get_width<float>(), image.get_height<float>());
	draw_pass(image.get_width<UINT>(), image.get_height<UINT>());
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);
}

void Renderer::pass_desigmoidize()
{
	const alignas(16) std::array data{
		Cb4{
			.x{ .f{ p_config->sigmoid_c }},
			.y{ .f{ p_config->sigmoid_m }}
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
	create_constant_buffer(cb0.ReleaseAndGetAddressOf(), sizeof(data));
	update_constant_buffer(cb0.Get(), data.data(), sizeof(data));
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_pixel_shader(PS_DESIGMOIDIZE, sizeof(PS_DESIGMOIDIZE));
	create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>());
	draw_pass(dims_output.width, dims_output.height);
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);
}

void Renderer::pass_blur()
{
	alignas(16) std::array cb0_data{
		Cb4{
			.x{ .f{ static_cast<float>(p_config->blur_r) }},
			.y{ .f{ p_config->blur_s }},
			.z{ .f{ 0.0f }}, //unsharp amount, must be 0.0f
		},
		Cb4{
			.x{ .f{ 0.0f }}, //x axis
			.y{ .f{ 1.0f }}, //y axis
			.z{ .f{ 0.0f }},
			.w{ .f{ 1.0f / image.get_height<float>() }}
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
	create_constant_buffer(cb0.ReleaseAndGetAddressOf(), sizeof(cb0_data));
	create_pixel_shader(PS_BLUR, sizeof(PS_BLUR));

	//pass y axis
	update_constant_buffer(cb0.Get(), cb0_data.data(), sizeof(cb0_data));
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_viewport(image.get_width<float>(), image.get_height<float>());
	draw_pass(image.get_width<UINT>(), image.get_height<UINT>());
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);

	//pass x axis
	cb0_data[1].x.f = 1.0f; // x axis
	cb0_data[1].y.f = 0.0f; // y axis
	cb0_data[1].z.f = 1.0f / image.get_width<float>();
	cb0_data[1].w.f = 0.0f;
	update_constant_buffer(cb0.Get(), cb0_data.data(), sizeof(cb0_data));
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_viewport(image.get_width<float>(), image.get_height<float>());
	draw_pass(image.get_width<UINT>(), image.get_height<UINT>());
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);
}

void Renderer::pass_unsharp()
{
	alignas(16) std::array cb0_data{
		Cb4{
			.x{ .f{ static_cast<float>(p_config->unsharp_r) }},
			.y{ .f{ p_config->unsharp_s }},
			.z{ .f{ p_config->unsharp_a }}, //unsharp amount, must be 0.0f
		},
		Cb4{
			.x{ .f{ 0.0f }}, //x axis
			.y{ .f{ 1.0f }}, //y axis
			.z{ .f{ 0.0f }},
			.w{ .f{ 1.0f / dims_output.get_height<float>() }}
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
	create_constant_buffer(cb0.ReleaseAndGetAddressOf(), sizeof(cb0_data));
	create_pixel_shader(PS_BLUR, sizeof(PS_BLUR));

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_original = srv_pass;

	//pass y axis
	update_constant_buffer(cb0.Get(), cb0_data.data(), sizeof(cb0_data));
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>());
	draw_pass(dims_output.width, dims_output.height);
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);

	//pass x axis
	cb0_data[1].x.f = 1.0f; //x axis
	cb0_data[1].y.f = 0.0f; //y axis
	cb0_data[1].z.f = 1.0f / dims_output.get_width<float>();
	cb0_data[1].w.f = 0.0f;
	update_constant_buffer(cb0.Get(), cb0_data.data(), sizeof(cb0_data));
	const std::array srvs{ srv_pass.Get(), srv_original.Get() };
	device_context->PSSetShaderResources(0, 2, srvs.data());
	create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>());
	draw_pass(dims_output.width, dims_output.height);
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);
}

void Renderer::pass_orthogonal_resample()
{
	alignas(16) std::array cb0_data{
		Cb4{
			.x{ .i{ p_config->kernel_i }},
			.y{ .f{ get_kernel_radius() }},
			.z{ .f{ p_config->kernel_b }},
			.w{ .f{ p_config->kernel_p1 }}
		},
		Cb4{
			.x{ .f{ p_config->kernel_p2 }},
			.y{ .f{ p_config->kernel_ar }},
			.z{ .f{ scale < 1.0f ? 1.0f / scale : 1.0f }},
		},
		Cb4{
			.x{ .f{ image.get_width<float>() }},
			.y{ .f{ image.get_height<float>() }},
			.z{ .f{ 0.0f }},
			.w{ .f{ 1.0f }}
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
	create_constant_buffer(cb0.ReleaseAndGetAddressOf(), sizeof(cb0_data));
	create_pixel_shader(PS_ORTHO, sizeof(PS_ORTHO));

	//pass y axis
	update_constant_buffer(cb0.Get(), cb0_data.data(), sizeof(cb0_data));
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_viewport(image.get_width<float>(), dims_output.get_height<float>());
	draw_pass(image.get_width<UINT>(), dims_output.height);
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);

	//pass x axis
	cb0_data[2].z.f = 1.0f; //x axis
	cb0_data[2].w.f = 0.0f; //y axis
	update_constant_buffer(cb0.Get(), cb0_data.data(), sizeof(cb0_data));
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>());
	draw_pass(dims_output.width, dims_output.height);
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);
}

void Renderer::pass_cylindrical_resample()
{
	const alignas(16) std::array data{
		Cb4{
			.x{ .i{ p_config->kernel_i }},
			.y{ .f{ get_kernel_radius() }},
			.z{ .f{ p_config->kernel_b }},
			.w{ .f{ p_config->kernel_p1 }}
		},
		Cb4{
			.x{ .f{ p_config->kernel_p2 }},
			.y{ .f{ p_config->kernel_ar }},
			.z{ .f{ image.get_width<float>() }},
			.w{ .f{ image.get_height<float>() }}
		},
		Cb4{
			.x{ .f{ scale < 1.0f ? 1.0f / scale : 1.0f }}
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
	create_constant_buffer(cb0.ReleaseAndGetAddressOf(), sizeof(data));
	update_constant_buffer(cb0.Get(), data.data(), sizeof(data));
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_pixel_shader(PS_CYL, sizeof(PS_CYL));
	create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>());
	draw_pass(dims_output.width, dims_output.height);
	
	//unbind render target
	device_context->OMSetRenderTargets(1, &(ID3D11RenderTargetView* const&)0, nullptr);
}

void Renderer::pass_last()
{
	if (image.has_alpha()) {
		const alignas(16) std::array cb0_data{
			Cb4{
				.x{ .f{ dims_output.get_width<float>() / p_config->alpha_t_size }},
				.y{ .f{ dims_output.get_height<float>() / p_config->alpha_t_size }}
			},
			Cb4{
				.x{ .f{ p_config->alpha_t1_c[0] }},
				.y{ .f{ p_config->alpha_t1_c[1] }},
				.z{ .f{ p_config->alpha_t1_c[2] }}
			},
			Cb4{
				.x{ .f{ p_config->alpha_t2_c[0] }},
				.y{ .f{ p_config->alpha_t2_c[1] }},
				.z{ .f{ p_config->alpha_t2_c[2] }}
			}
		};
		Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
		create_constant_buffer(cb0.ReleaseAndGetAddressOf(), sizeof(cb0_data));
		update_constant_buffer(cb0.Get(), cb0_data.data(), sizeof(cb0_data));
		create_pixel_shader(PS_SAMPLE_ALPHA, sizeof(PS_SAMPLE_ALPHA));
	}
	else
		create_pixel_shader(PS_SAMPLE, sizeof(PS_SAMPLE));
	device_context->PSSetShaderResources(0, 1, srv_pass.GetAddressOf());
	create_viewport(dims_output.get_width<float>(), dims_output.get_height<float>(), true);
}

void Renderer::draw_pass(UINT width, UINT height) noexcept
{
	//cereate texture
	const D3D11_TEXTURE2D_DESC texture2d_desc{
		.Width{ width },
		.Height{ height },
		.MipLevels{ 1 },
		.ArraySize{ 1 },
		.Format{ DXGI_FORMAT_R32G32B32A32_FLOAT },
		.SampleDesc{
			.Count{ 1 }
		},
		.BindFlags{ D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET }
	};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
	wiv_assert(device->CreateTexture2D(&texture2d_desc, nullptr, texture2d.ReleaseAndGetAddressOf()), == S_OK);

	//create render target view
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
	wiv_assert(device->CreateRenderTargetView(texture2d.Get(), nullptr, rtv.ReleaseAndGetAddressOf()), == S_OK);

	//draw
	device_context->OMSetRenderTargets(1, rtv.GetAddressOf(), nullptr);
	static constexpr std::array clear_color{ 0.0f, 0.0f, 0.0f, 1.0f };
	device_context->ClearRenderTargetView(rtv.Get(), clear_color.data());
	device_context->Draw(3, 0);

	//create shader resource view
	wiv_assert(device->CreateShaderResourceView(texture2d.Get(), nullptr, srv_pass.ReleaseAndGetAddressOf()), == S_OK);
}

void Renderer::create_constant_buffer(ID3D11Buffer** buffer, UINT byte_width) const noexcept
{
	const D3D11_BUFFER_DESC buffer_desc{
		.ByteWidth{ byte_width },
		.Usage{ D3D11_USAGE_DYNAMIC },
		.BindFlags{ D3D11_BIND_CONSTANT_BUFFER },
		.CPUAccessFlags{ D3D11_CPU_ACCESS_WRITE },
	};
	wiv_assert(device->CreateBuffer(&buffer_desc, 0, buffer), == S_OK);
	device_context->PSSetConstantBuffers(0, 1, buffer);
}

void Renderer::update_constant_buffer(ID3D11Buffer* buffer, const void* data, size_t size) const noexcept
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource;
	wiv_assert(device_context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource), == S_OK);
	std::memcpy(mapped_subresource.pData, data, size);
	device_context->Unmap(buffer, 0);
}

void Renderer::create_pixel_shader(const BYTE* shader, size_t shader_size) const noexcept
{
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	wiv_assert(device->CreatePixelShader(shader, shader_size, nullptr, pixel_shader.ReleaseAndGetAddressOf()), == S_OK);
	device_context->PSSetShader(pixel_shader.Get(), nullptr, 0);
}

void Renderer::create_viewport(float width, float height, bool adjust) const noexcept
{
	D3D11_VIEWPORT viewport{
		.Width{ width },
		.Height{ height }
	};

	//offset image in order to center it in the window
	if (adjust) {
		if (get_ratio<double>(dims_swap_chain.width, dims_swap_chain.height) > get_ratio<double>(width, height)) {
			viewport.TopLeftX = (dims_swap_chain.width - viewport.Width) / 2.0f;
		}
		else {
			viewport.TopLeftY = (dims_swap_chain.height - viewport.Height) / 2.0f;
		}
	}
	
	device_context->RSSetViewports(1, &viewport);
}

void Renderer::update_dims_output()
{
	if (get_ratio<double>(dims_swap_chain.width, dims_swap_chain.height) > get_ratio<double>(image.get_width<double>(), image.get_height<double>())) {
		dims_output.width = static_cast<int>(std::lround(image.get_width<double>() * get_ratio<double>(dims_swap_chain.height, image.get_height<double>())));
		dims_output.height = dims_swap_chain.height;
	}
	else {
		dims_output.width = dims_swap_chain.width;
		dims_output.height = static_cast<int>(std::lround(image.get_height<double>() * get_ratio<double>(dims_swap_chain.width, image.get_width<double>())));
	}
}

void Renderer::update_scale() noexcept
{
	const auto x_ratio{ get_ratio<float>(dims_output.width, image.get_width<float>()) };
	const auto y_ratio{ get_ratio<float>(dims_output.height, image.get_height<float>()) };

	//downscale
	if (x_ratio < 1.0f || y_ratio < 1.0f)
		scale = std::min(x_ratio, y_ratio);

	//upscale
	else if (x_ratio > 1.0f || y_ratio > 1.0f)
		scale = std::max(x_ratio, y_ratio);
	
	//no scaling
	else
		scale = 1.0f;
}

float Renderer::get_kernel_radius() const noexcept
{
	if (p_config->kernel_i == WIV_KERNEL_FUNCTION_NEAREST || (p_config->kernel_i == WIV_KERNEL_FUNCTION_LINEAR && !p_config->kernel_use_cyl))
		return 1.0f;
	else if (p_config->kernel_i == WIV_KERNEL_FUNCTION_LINEAR && p_config->kernel_use_cyl)
		return std::numbers::sqrt2_v<float>;
	else if (p_config->kernel_i == WIV_KERNEL_FUNCTION_BICUBIC || p_config->kernel_i == WIV_KERNEL_FUNCTION_FSR || p_config->kernel_i == WIV_KERNEL_FUNCTION_BCSPLINE)
		return 2.0f;
	return p_config->kernel_r;
}

void Renderer::update_tone_responce_curve()
{
	if (p_config->cms_use) {
		if ((p_config->cms_profile_display == WIV_CMS_PROFILE_DISPLAY_AUTO || p_config->cms_profile_display == WIV_CMS_PROFILE_DISPLAY_CUSTOM) && cms_profile_display)
			trc = { WIV_CMS_TRC_GAMMA, static_cast<float>(cmsDetectRGBProfileGamma(cms_profile_display.get(), 0.1)) };
		else if (p_config->cms_profile_display == WIV_CMS_PROFILE_DISPLAY_ADOBE)
			trc = { WIV_CMS_TRC_GAMMA, 2.19921875f /* source https://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf */ };
		else if (p_config->cms_profile_display == WIV_CMS_PROFILE_DISPLAY_SRGB)
			trc = { WIV_CMS_TRC_SRGB, 0.0f /* will be ignored */ };
	}
	else if (image.embended_profile)
		trc = { WIV_CMS_TRC_GAMMA, static_cast<float>(cmsDetectRGBProfileGamma(image.embended_profile.get(), 0.1)) };
	else if (image.get_tagged_color_space() == WIV_COLOR_SPACE_ADOBE)
		trc = { WIV_CMS_TRC_GAMMA, 2.19921875f /* source https://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf */ };
	else if (image.get_tagged_color_space() == WIV_COLOR_SPACE_SRGB || p_config->cms_use_defualt_to_srgb)
		trc = { WIV_CMS_TRC_SRGB, 1.0f /* will be ignored */ };
	else
		trc = { WIV_CMS_TRC_NONE, 0.0f };
}

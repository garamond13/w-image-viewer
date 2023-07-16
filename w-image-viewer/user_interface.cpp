#include "pch.h"
#include "user_interface.h"
#include "global.h"
#include "font.h"
#include "helpers.h"
#include "shader_config.h"
#include "version.h"
#include "supported_extensions.h"

User_interface::~User_interface()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void User_interface::create(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* device_context, bool* should_update)
{
	this->hwnd = hwnd;
	p_renderer_should_update = should_update;

#ifndef NDEBUG
	IMGUI_CHECKVERSION();
#endif

	ImGui::CreateContext();

	//font
	ImVector<ImWchar> ranges;
	ImFontGlyphRangesBuilder builder;
	builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesDefault());
	builder.AddChar(0x0161); // š
	builder.BuildRanges(&ranges);
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(ROBOTO_MEDIUM_COMPRESSED_DATA, ROBOTO_MEDIUM_COMPRESSED_SIZE, 16, nullptr, ranges.Data);
	ImGui::GetIO().Fonts->Build();
	
	ImGui::GetIO().IniFilename = nullptr;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device, device_context);
}

void User_interface::update()
{
	//feed inputs to dear imgui, start new frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	input();
	context_menu();
	window_settings();
	window_about();

	//DEBUG ONLY!
	//

#if 0
	ImGui::ShowMetricsWindow();
#endif

#if 0
	ImGui::ShowDemoWindow();
#endif

	//

	ImGui::Render();
}

void User_interface::draw() const
{
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void User_interface::auto_window_size() const
{
	if (is_fullscreen || IsZoomed(hwnd))
		return;

	//get the screen width and height
	const auto cx_screen{ static_cast<double>(GetSystemMetrics(SM_CXVIRTUALSCREEN)) };
	const auto cy_screen{ static_cast<double>(GetSystemMetrics(SM_CYVIRTUALSCREEN)) };
	
	const auto image_width{ file_manager.image.get_width<double>() };
	const auto image_height{ file_manager.image.get_height<double>() };
	RECT rect{};

	//if the image resolution is larger than the screen resolution * 0.9, downsize the window to screen resolution * 0.9 with the aspect ratio of the image
	if (cx_screen / cy_screen > image_width / image_height) {
		rect.right = image_width > cx_screen * 0.9 ? std::lround(image_width * cy_screen / image_height * 0.9) : static_cast<LONG>(image_width);
		rect.bottom = image_height > cy_screen * 0.9 ? std::lround(image_height * rect.right / image_width) : static_cast<LONG>(image_height);
	}
	else {
		rect.bottom = image_height > cy_screen * 0.9 ? std::lround(image_height * cx_screen / image_width * 0.9) : static_cast<LONG>(image_height);
		rect.right = image_width > cx_screen * 0.9 ? std::lround(image_width * rect.bottom / image_height) : static_cast<LONG>(image_width);
	}

	AdjustWindowRectEx(&rect, WIV_WINDOW_STYLE, FALSE, WIV_WINDOW_EX_STYLE);

	//calculate window width and height
	const auto cx{ rect.right - rect.left };
	const auto cy{ rect.bottom - rect.top };

	//center the window and apply new dimensions
	SetWindowPos(hwnd, nullptr, static_cast<int>((cx_screen - cx) / 2.0), static_cast<int>((cy_screen - cy) / 2.0), cx, cy, SWP_NOZORDER);
}

void User_interface::reset_image_panzoom() noexcept
{
	image_pan = ImVec2();
	image_zoom = 0.0f;
	image_no_scale = false;
}

void User_interface::input()
{
	//workaround for IsMouseDoubleClicked(0) triggering IsMouseDragging(0)
	static bool is_double_click;
	if (ImGui::IsMouseReleased(0))
		is_double_click = false;

	//mouse
	if (!ImGui::GetIO().WantCaptureMouse) {
		if (ImGui::IsMouseDoubleClicked(0)) {
			toggle_fullscreen();
			is_double_click = true;
			return;
		}

		//image panning
		if (!is_double_click && ImGui::IsMouseDragging(0)) {
			image_pan += ImGui::GetMouseDragDelta();
			ImGui::ResetMouseDragDelta();
			is_panning = true;
			*p_renderer_should_update = true;
			return;
		}

		//image zoom
		if (ImGui::GetIO().MouseWheel > 0.0f) {
			image_zoom += 0.1f;
			is_zooming = true;
			*p_renderer_should_update = true;
			return;
		}
		if (ImGui::GetIO().MouseWheel < 0.0f) {
			image_zoom -= 0.1f;
			is_zooming = true;
			*p_renderer_should_update = true;
			return;
		}
	}

	//keyboard
	if (!ImGui::GetIO().WantCaptureKeyboard) {

		//ctrl + key
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
			if (ImGui::IsKeyPressed(ImGuiKey_O, false)) {
				
				//it will block the thread, but probably no need to run it from new thread
				dialog_file_open(WIV_OPEN_IMAGE);
				
				return;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_1, false)) {
				reset_image_panzoom();
				image_no_scale = true;
				*p_renderer_should_update = true;
				return;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_0, false)) {
				reset_image_panzoom();
				*p_renderer_should_update = true;
				return;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) {
				image_zoom += 0.1f;
				is_zooming = true;
				*p_renderer_should_update = true;
				return;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) {
				image_zoom -= 0.1f;
				is_zooming = true;
				*p_renderer_should_update = true;
				return;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
				image_rotation -= 90.0f;
				is_rotating = true;
				*p_renderer_should_update = true;
				return;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
				image_rotation += 90.0f;
				is_rotating = true;
				*p_renderer_should_update = true;
				return;
			}
		}

		//alt + key
		if (ImGui::IsKeyDown(ImGuiKey_LeftAlt)) {
			if (ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
				toggle_fullscreen();
				return;
			}
		}

		//free keys
		if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
			if (file_manager.file_current.empty())
				return;
			file_manager.file_previous();
			wiv_assert(PostMessageW(hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
			return;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
			if (file_manager.file_current.empty())
				return;
			file_manager.file_next();
			wiv_assert(PostMessageW(hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
			return;
		}
	}

	//leave escape key with higher priority
	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
		if (is_fullscreen)
			toggle_fullscreen();
		else
			wiv_assert(DestroyWindow(hwnd), != 0);
		return;
	}
}

void User_interface::context_menu()
{
	if (ImGui::BeginPopupContextVoid("Right click menu")) {
		if (ImGui::Selectable("Open...")) {
			std::thread t(&User_interface::dialog_file_open, this, WIV_OPEN_IMAGE);
			t.detach();
			goto end;
		}
		ImGui::Separator();
		if (ImGui::Selectable("Next")) {
			if (file_manager.file_current.empty())
				goto end;
			file_manager.file_next();
			wiv_assert(PostMessageW(hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
			goto end;
		}
		if (ImGui::Selectable("Previous")) {
			if (file_manager.file_current.empty())
				goto end;
			file_manager.file_previous();
			wiv_assert(PostMessageW(hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
			goto end;
		}
		ImGui::Separator();
		if (ImGui::Selectable("Fullscreen")) {
			toggle_fullscreen();
			goto end;
		}
		ImGui::Separator();
		if (ImGui::Selectable("Rotate CW")) {
			image_rotation += 90.0f;
			is_rotating = true;
			*p_renderer_should_update = true;
			goto end;
		}
		if (ImGui::Selectable("Rotate CCW")) {
			image_rotation -= 90.0f;
			is_rotating = true;
			*p_renderer_should_update = true;
			goto end;
		}
		ImGui::Separator();
		if (ImGui::Selectable("Settings...")) {
			is_window_settings_open = true;
			goto end;
		}
		if (ImGui::Selectable("About...")) {
			is_window_about_open = true;
			goto end;
		}
		ImGui::Separator();
		if (ImGui::Selectable("Exit")) {
			wiv_assert(DestroyWindow(hwnd), != 0);
			goto end;
		}
	end:
		ImGui::EndPopup();
	}
}

void User_interface::window_settings()
{
	if (is_window_settings_open) {
		static constinit const ImVec2 window_size{ 430.0f, 430.0f };
		static constinit const ImVec2 button_size{ -1.0f, 0.0f };
		ImGui::SetNextWindowSize(window_size, ImGuiCond_Once);
		ImGui::Begin("Settings", &is_window_settings_open, ImGuiWindowFlags_NoCollapse);
		if (ImGui::CollapsingHeader("General")) {
			ImGui::Spacing();
			ImGui::TextUnformatted("Default window dimensions:");
			ImGui::InputInt("Width", &g_config.window_width, 0, 0);
			ImGui::InputInt("Height", &g_config.window_height, 0, 0);
			ImGui::Spacing();
			ImGui::Checkbox("Enable window auto dimensions", &g_config.window_autowh);
			ImGui::Spacing();
			ImGui::ColorEdit4("Background color", g_config.clear_color.data(), ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHSV);
			ImGui::Spacing();
			static constinit const std::array items{
				"Defualt name",
				"Filename",
				"Full filename"
			};
			ImGui::Combo("Window name", &g_config.window_name, items.data(), items.size());
			ImGui::Spacing();
			static constinit const std::array items2{
				"RGBA32F",
				"RGBA16F"
			};
			ImGui::Combo("Internal format", &g_config.pass_format, items2.data(), items2.size());
			ImGui::Spacing();
			ImGui::Checkbox("Read only thumbnail in RAW image", &g_config.raw_thumb);
			ImGui::Spacing();
		}
		if (ImGui::CollapsingHeader("Scale")) {
			ImGui::Spacing();

			//scale profiles
			//

			ImGui::TextUnformatted("Profiles:");
			static Range<float> range;
			std::array range_array{ &range.lower, &range.upper };
			ImGui::InputFloat2("##range", *range_array.data(), "%.6f");
			range.clamp();
			ImGui::SameLine();
			if (ImGui::Button("Add profile", button_size)) {

				//check first does profile already exists
				bool exists{};
				for (const auto& profile : g_config.scale_profiles) {
					if (profile.range == range)
						exists = true;
				}

				if (!exists)
					g_config.scale_profiles.push_back({ range, {} });
			}

			//hold values
			//cant use vector<const char*> directly, because const char* will be non owning
			std::vector<std::string> profile_names;
			for (const auto& profile_name : g_config.scale_profiles)
				profile_names.push_back(std::to_string(profile_name.range.lower) + ", " + std::to_string(profile_name.range.upper));

			//cant use string directly in imgui
			std::vector<const char*> profile_items;
			for (const auto& profile_item : profile_names)
				profile_items.push_back(profile_item.c_str());

			static int current_profile;
			ImGui::Combo("##profile", &current_profile, profile_items.data(), profile_items.size());
			ImGui::SameLine();
			if (ImGui::Button("Remove profile", button_size)) {
				if (current_profile > 0) {
					g_config.scale_profiles.erase(g_config.scale_profiles.begin() + current_profile);
					--current_profile;
				}
			}
			auto& scale{ g_config.scale_profiles[current_profile].config };
			ImGui::Spacing();

			//

			//pre-scale blur
			ImGui::SeparatorText("Pre-scale blur (downscale only)");
			ImGui::Checkbox("Enable pre-scale blur", &scale.blur_use);
			dimm(!scale.blur_use);
			ImGui::InputInt("Radius##blur", &scale.blur_radius, 0, 0);
			ImGui::InputFloat("Sigma##blur", &scale.blur_sigma, 0.0f, 0.0f, "%.6f");
			dimm();
			ImGui::Spacing();

			//sigmoidize
			ImGui::SeparatorText("Sigmoidize (upscale only)");
			ImGui::Checkbox("Enable sigmoidize", &scale.sigmoid_use);
			dimm(!scale.sigmoid_use);
			ImGui::InputFloat("Contrast", &scale.sigmoid_contrast, 0.0f, 0.0f, "%.6f");
			ImGui::InputFloat("Midpoint", &scale.sigmoid_midpoint, 0.0f, 0.0f, "%.6f");
			dimm();
			ImGui::Spacing();

			//scale
			ImGui::SeparatorText("Scale");
			ImGui::Checkbox("Use cylindrical filtering", &scale.kernel_use_cyl);
			ImGui::Spacing();
			static constinit const std::array items{
				"Lanczos",
				"Ginseng",
				"Hamming",
				"Power of cosine",
				"Garamond-Kaiser",
				"Power of Garamond",
				"Power of Blackman",
				"GNW",
				"Said",
				"Nearest neighbor",
				"Linear",
				"Bicubic",
				"Modified FSR",
				"BC-Spline"
			};
			ImGui::Combo("Kernel-function", &scale.kernel_index, items.data(), items.size());
			const auto& i{ scale.kernel_index };
			dimm(i == WIV_KERNEL_FUNCTION_NEAREST || i == WIV_KERNEL_FUNCTION_LINEAR || i == WIV_KERNEL_FUNCTION_BICUBIC || i == WIV_KERNEL_FUNCTION_FSR || i == WIV_KERNEL_FUNCTION_BCSPLINE);
			ImGui::InputFloat("Radius##kernel", &scale.kernel_radius, 0.0f, 0.0f, "%.6f");
			ImGui::InputFloat("Blur", &scale.kernel_blur, 0.0f, 0.0f, "%.6f");
			dimm();
			dimm(i == WIV_KERNEL_FUNCTION_LANCZOS || i == WIV_KERNEL_FUNCTION_GINSENG || i == WIV_KERNEL_FUNCTION_HAMMING || i == WIV_KERNEL_FUNCTION_NEAREST || i == WIV_KERNEL_FUNCTION_LINEAR);
			ImGui::InputFloat("Parameter 1", &scale.kernel_p1, 0.0f, 0.0f, "%.6f");
			dimm();
			dimm(i == WIV_KERNEL_FUNCTION_LANCZOS || i == WIV_KERNEL_FUNCTION_GINSENG || i == WIV_KERNEL_FUNCTION_HAMMING || i == WIV_KERNEL_FUNCTION_POW_COSINE || i == WIV_KERNEL_FUNCTION_NEAREST || i == WIV_KERNEL_FUNCTION_LINEAR || i == WIV_KERNEL_FUNCTION_BICUBIC);
			ImGui::InputFloat("Parameter 2", &scale.kernel_p2, 0.0f, 0.0f, "%.6f");
			dimm();
			ImGui::InputFloat("Anti-ringing", &scale.kernel_ar, 0.0f, 0.0f, "%.6f");
			ImGui::Spacing();

			//post-scale unsharp
			ImGui::SeparatorText("Post-scale unsharp mask");
			ImGui::Checkbox("Enable post-scale unsharp mask", &scale.unsharp_use);
			dimm(!scale.unsharp_use);
			ImGui::InputInt("Radius##unsharp", &scale.unsharp_radius, 0, 0);
			ImGui::InputFloat("Sigma##unsharp", &scale.unsharp_sigma, 0.0f, 0.0f, "%.6f");
			ImGui::InputFloat("Amount", &scale.unsharp_amount, 0.0f, 0.0f, "%.6f");
			dimm();
			ImGui::Spacing();
		}
		if (ImGui::CollapsingHeader("Color managment")) {
			ImGui::Spacing();
			ImGui::Checkbox("Enable color managment system", &g_config.cms_use);
			ImGui::Spacing();
			static constinit const std::array items2{
				"Auto",
				"sRGB",
				"AdobeRGB",
				"Custom"
			};
			ImGui::Combo("Display profile", &g_config.cms_profile_display, items2.data(), items2.size());
			char buffer[MAX_PATH];
			std::strcpy(buffer, g_config.cms_profile_display_custom.string().c_str());
			dimm(g_config.cms_profile_display != WIV_CMS_PROFILE_DISPLAY_CUSTOM);
			ImGui::InputText("##custom path", buffer, MAX_PATH);
			g_config.cms_profile_display_custom = buffer;
			dimm();
			ImGui::SameLine();
			if (ImGui::Button("Custom...", button_size)) {
				std::thread t(&User_interface::dialog_file_open, this, WIV_OPEN_ICC);
				t.detach();
			}
			ImGui::Spacing();
			static constinit const std::array items{
				"Perceptual",
				"Relative colorimetric",
				"Saturation",
				"Absolute colorimetric"
			};
			ImGui::Combo("Rendering intent", &g_config.cms_intent, items.data(), items.size());
			ImGui::Spacing();
			ImGui::Checkbox("Enable black point compensation", &g_config.cms_use_bpc);
			ImGui::SeparatorText("Untagged images");
			ImGui::Checkbox("Default to sRGB", &g_config.cms_use_defualt_to_srgb);
			ImGui::Spacing();
		}
		if (ImGui::CollapsingHeader("Transparency")) {
			ImGui::Spacing();
			ImGui::InputFloat("Tile size", &g_config.alpha_tile_size, 0.0f, 0.0f, "%.6f");
			ImGui::Spacing();
			ImGui::ColorEdit3("First tile color", g_config.alpha_tile1_color.data(), ImGuiColorEditFlags_DisplayHSV);
			ImGui::ColorEdit3("Second tile color", g_config.alpha_tile2_color.data(), ImGuiColorEditFlags_DisplayHSV);
		}
		ImGui::SeparatorText("Changes");
		if (ImGui::Button("Write changes")) {
			g_config.write();
		}
		ImGui::Spacing();
		ImGui::End();
	}
}

void User_interface::window_about()
{
	if (is_window_about_open) {
		static constinit const ImVec2 size{ 288, 178.0f };
		ImGui::SetNextWindowSize(size);
		ImGui::Begin("About", &is_window_about_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("W Image Viewer %d.%d.%d", WIV_VERSION_NUMBER_MAJOR, WIV_VERSION_NUMBER_MINOR, WIV_VERSION_NUMBER_PATCH);
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::TextUnformatted(reinterpret_cast<const char*>(u8"Created by Ivan Bjeliš."));
		ImGui::Spacing();
		ImGui::Spacing();
		if (ImGui::Button("Web page...")) {
			if (is_fullscreen)
				toggle_fullscreen();
			ShellExecuteW(nullptr, L"open", L"https://github.com/garamond13/w-image-viewer", nullptr, nullptr, SW_SHOWNORMAL);
		}
		ImGui::End();
	}
}

//dont think this can get any uglier
void User_interface::dialog_file_open(WIV_OPEN_ file_type)
{
	is_dialog_file_open = true;
	wiv_assert(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE), >= S_OK);
	Microsoft::WRL::ComPtr<IFileOpenDialog> file_open_dialog;
	if (CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(file_open_dialog.ReleaseAndGetAddressOf())) == S_OK) {
		COMDLG_FILTERSPEC filterspec{
			.pszName{ L"All supported" }
		};
		if (file_type == WIV_OPEN_IMAGE)
			filterspec.pszSpec = WIV_SUPPORTED_EXTENSIONS;
		else //WIV_OPEN_ICC
			filterspec.pszSpec = L"*.icc";
		wiv_assert(file_open_dialog->SetFileTypes(1, &filterspec), == S_OK);
		if (file_open_dialog->Show(hwnd) == S_OK) {
			Microsoft::WRL::ComPtr<IShellItem> shell_item;
			if (file_open_dialog->GetResult(shell_item.ReleaseAndGetAddressOf()) == S_OK) {
				wchar_t* path;
				if (shell_item->GetDisplayName(SIGDN_FILESYSPATH, &path) == S_OK) {
					if (file_type == WIV_OPEN_IMAGE) {
						if (file_manager.file_open(path))
							wiv_assert(PostMessageW(hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
						else
							wiv_assert(PostMessageW(hwnd, WIV_WM_RESET_RESOURCES, 0, 0), != 0);
					}
					else //WIV_OPEN_ICC
						g_config.cms_profile_display_custom = path;
					CoTaskMemFree(path);
				}
			}
		}
	}
	CoUninitialize();
	is_dialog_file_open = false;
}

void User_interface::toggle_fullscreen()
{
	static RECT rect;
	static bool is_maximized;
	if (is_fullscreen) {
		SetWindowLongPtrW(hwnd, GWL_STYLE, WIV_WINDOW_STYLE);
		wiv_assert(SetWindowPos(hwnd, HWND_NOTOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE), != 0);
		ShowWindow(hwnd, is_maximized ? SW_MAXIMIZE : SW_SHOW);
	}
	else {
		is_maximized = IsZoomed(hwnd);
		wiv_assert(GetWindowRect(hwnd, &rect), != 0);
		SetWindowLongPtrW(hwnd, GWL_STYLE, WS_POPUP);
		wiv_assert(SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE), != 0);
		ShowWindow(hwnd, SW_MAXIMIZE);
	}
	is_fullscreen = !is_fullscreen;
}

//imgui dimming helper
void User_interface::dimm(bool condition) const
{
	static bool is_pushed;
	if (condition) {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.45f);
		is_pushed = true;
	}
	else if (is_pushed) {
		ImGui::PopStyleVar();
		is_pushed = false;
	}
}

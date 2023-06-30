#include "pch.h"
#include "user_interface.h"
#include "font.h"
#include "helpers.h"
#include "shader_config.h"
#include "version.h"

User_interface::~User_interface()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void User_interface::create(Config* p_config, HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* device_context, bool* should_update)
{
	this->p_config = p_config;
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

void User_interface::input()
{
	//workaround for IsMouseDoubleClicked(0) triggering IsMouseDragging(0)
	static bool is_double_click;
	if (ImGui::IsMouseReleased(0))
		is_double_click = false;
	
	if (!ImGui::GetIO().WantCaptureMouse) {
		if (ImGui::IsMouseDoubleClicked(0)) {
			toggle_fullscreen();
			is_double_click = true;
			return;
		}
		
		//image panning
		if (!is_double_click && ImGui::IsMouseDragging(0)) {
			const auto delta{ ImGui::GetMouseDragDelta() };
			image_pan.first += delta.x;
			image_pan.second += delta.y;
			ImGui::ResetMouseDragDelta();
			*p_renderer_should_update = true;
			return;
		}

		//image zooming
		if (ImGui::GetIO().MouseWheel > 0.0f) {
			image_zoom += 0.1f;
			*p_renderer_should_update = true;
			return;
		}
		if (ImGui::GetIO().MouseWheel < 0.0f) {
			image_zoom -= 0.1f;
			if (image_zoom < 0.1)
				image_zoom = 0.1;
			*p_renderer_should_update = true;
			return;
		}
	}

	if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && !ImGui::GetIO().WantCaptureKeyboard) {
		if (file_manager.file_current.empty())
			return;
		file_manager.file_previous();
		wiv_assert(PostMessageW(hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
		return;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && !ImGui::GetIO().WantCaptureKeyboard) {
		if (file_manager.file_current.empty())
			return;
		file_manager.file_next();
		wiv_assert(PostMessageW(hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
		return;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
		if (is_fullscreen)
			toggle_fullscreen();
		else
			wiv_assert(DestroyWindow(hwnd), != 0);
		return;
	}
	if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
		toggle_fullscreen();
		return;
	}
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_O, false)) {
		dialog_file_open();
		return;
	}
}

void User_interface::context_menu()
{
	if (ImGui::BeginPopupContextVoid("Right click menu")) {
		if (ImGui::Selectable("Open...")) {
			std::thread t(&User_interface::dialog_file_open, this);
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
		constexpr ImVec2 size{ 430.0f, 430.0f };
		ImGui::SetNextWindowSize(size, ImGuiCond_Once);
		ImGui::Begin("Settings", &is_window_settings_open, ImGuiWindowFlags_NoCollapse);
		if (ImGui::CollapsingHeader("General")) {
			ImGui::Spacing();
			ImGui::TextUnformatted("Default window dimensions:");
			ImGui::InputInt("Width", &p_config->window_w, 0, 0);
			ImGui::InputInt("Height", &p_config->window_h, 0, 0);
			ImGui::Spacing();
			ImGui::Checkbox("Enable window auto dimensions", &p_config->window_autowh);
			ImGui::Spacing();
			ImGui::ColorEdit4("Background color", p_config->clear_c.data(), ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHSV);
			ImGui::Spacing();
			constexpr std::array items{
				"RGBA32F",
				"RGBA16F"
			};
			ImGui::Combo("Internal format", &p_config->pass_format, items.data(), items.size());
			ImGui::Spacing();
		}
		if (ImGui::CollapsingHeader("Scale")) {
			
			//pre-scale blur
			ImGui::SeparatorText("Pre-scale blur (downscale only)");
			ImGui::Checkbox("Enable pre-scale blur", &p_config->blur_use);
			dimm(!p_config->blur_use);
			ImGui::InputInt("Radius##blur", &p_config->blur_r, 0, 0);
			ImGui::InputFloat("Sigma##blur", &p_config->blur_s, 0.0f, 0.0f, "%.6f");
			dimm();
			ImGui::Spacing();

			//sigmoidize
			ImGui::SeparatorText("Sigmoidize (upscale only)");
			ImGui::Checkbox("Enable sigmoidize", &p_config->sigmoid_use);
			dimm(!p_config->sigmoid_use);
			ImGui::InputFloat("Contrast", &p_config->sigmoid_c, 0.0f, 0.0f, "%.6f");
			ImGui::InputFloat("Midpoint", &p_config->sigmoid_m, 0.0f, 0.0f, "%.6f");
			dimm();
			ImGui::Spacing();

			//scale
			ImGui::SeparatorText("Scale");
			ImGui::Checkbox("Use cylindrical filtering", &p_config->kernel_use_cyl);
			ImGui::Spacing();
			constexpr std::array items{
				"lanczos",
				"ginseng",
				"hamming",
				"power of cosine",
				"garamond-kaiser",
				"power of garamond",
				"power of blackman",
				"gnw",
				"said",
				"nearest neighbor",
				"linear",
				"bicubic",
				"modified fsr kernel",
				"bc-spline"
			};
			ImGui::Combo("Kernel-function", &p_config->kernel_i, items.data(), items.size());
			const auto& i{ p_config->kernel_i };
			dimm(i == WIV_KERNEL_FUNCTION_NEAREST || i == WIV_KERNEL_FUNCTION_LINEAR || i == WIV_KERNEL_FUNCTION_BICUBIC || i == WIV_KERNEL_FUNCTION_FSR || i == WIV_KERNEL_FUNCTION_BCSPLINE);
			ImGui::InputFloat("Radius##kernel", &p_config->kernel_r, 0.0f, 0.0f, "%.6f");
			ImGui::InputFloat("Blur", &p_config->kernel_b, 0.0f, 0.0f, "%.6f");
			dimm();
			dimm(i == WIV_KERNEL_FUNCTION_LANCZOS || i == WIV_KERNEL_FUNCTION_GINSENG || i == WIV_KERNEL_FUNCTION_HAMMING || i == WIV_KERNEL_FUNCTION_NEAREST || i == WIV_KERNEL_FUNCTION_LINEAR);
			ImGui::InputFloat("Parameter1", &p_config->kernel_p1, 0.0f, 0.0f, "%.6f");
			dimm();
			dimm(i == WIV_KERNEL_FUNCTION_LANCZOS || i == WIV_KERNEL_FUNCTION_GINSENG || i == WIV_KERNEL_FUNCTION_HAMMING || i == WIV_KERNEL_FUNCTION_POW_COSINE || i == WIV_KERNEL_FUNCTION_NEAREST || i == WIV_KERNEL_FUNCTION_LINEAR || i == WIV_KERNEL_FUNCTION_BICUBIC);
			ImGui::InputFloat("Parameter2", &p_config->kernel_p2, 0.0f, 0.0f, "%.6f");
			dimm();
			ImGui::InputFloat("Anti-ringing", &p_config->kernel_ar, 0.0f, 0.0f, "%.6f");
			ImGui::Spacing();

			//post-scale unsharp
			ImGui::SeparatorText("Post-scale unsharp mask");
			ImGui::Checkbox("Enable post-scale unsharp mask", &p_config->unsharp_use);
			dimm(!p_config->unsharp_use);
			ImGui::InputInt("Radius##unsharp", &p_config->unsharp_r, 0, 0);
			ImGui::InputFloat("Sigma##unsharp", &p_config->unsharp_s, 0.0f, 0.0f, "%.6f");
			ImGui::InputFloat("Amount", &p_config->unsharp_a, 0.0f, 0.0f, "%.6f");
			dimm();
			ImGui::Spacing();
		}
		if (ImGui::CollapsingHeader("Color managment")) {
			ImGui::SeparatorText("Color managment system");
			ImGui::Checkbox("Enable color managment system", &p_config->cms_use);
			ImGui::Spacing();
			constexpr std::array items2{
				"auto",
				"sRGB",
				"AdobeRGB1998",
				"custom"
			};
			ImGui::Combo("Display profile", &p_config->cms_profile_display, items2.data(), items2.size());
			char buffer[MAX_PATH];
			std::strcpy(buffer, p_config->cms_profile_display_custom.string().c_str());
			dimm(p_config->cms_profile_display != WIV_CMS_PROFILE_DISPLAY_CUSTOM);
			ImGui::InputText("Custom path", buffer, MAX_PATH);
			p_config->cms_profile_display_custom = buffer;
			dimm();
			ImGui::Spacing();
			constexpr std::array items{
				"perceptual",
				"relative colorimetric",
				"saturation",
				"absolute colorimetric"
			};
			ImGui::Combo("Rendering intent", &p_config->cms_intent, items.data(), items.size());
			ImGui::Spacing();
			ImGui::Checkbox("Enable black point compensation", &p_config->cms_use_bpc);
			ImGui::SeparatorText("Untagged images");
			ImGui::Checkbox("Default to sRGB", &p_config->cms_use_defualt_to_srgb);
			ImGui::Spacing();
		}
		if (ImGui::CollapsingHeader("Transparency")) {
			ImGui::Spacing();
			ImGui::InputFloat("Tile size", &p_config->alpha_t_size, 0.0f, 0.0f, "%.6f");
			ImGui::Spacing();
			ImGui::ColorEdit3("First tile color", p_config->alpha_t1_c.data(), ImGuiColorEditFlags_DisplayHSV);
			ImGui::ColorEdit3("Second tile color", p_config->alpha_t2_c.data(), ImGuiColorEditFlags_DisplayHSV);
		}
		ImGui::SeparatorText("Changes");
		if (ImGui::Button("Reset changes"))
			p_config->map_values();
		ImGui::SameLine();
		if (ImGui::Button("Write changes")) {
			p_config->map_values();
			p_config->file_write();
		}
		ImGui::Spacing();
		ImGui::End();
	}
}

void User_interface::window_about()
{
	if (is_window_about_open) {
		constexpr ImVec2 size{ 288, 178.0f };
		ImGui::SetNextWindowSize(size);
		ImGui::Begin("About", &is_window_about_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("W Image Viewer %d.%d.%d", WIV_VERSION_NUMBER_MAJOR, WIV_VERSION_NUMBER_MINOR, WIV_VERSION_NUMBER_PATCH);
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::TextUnformatted(reinterpret_cast<const char*>(u8"Created by Ivan Bjeliš."));
		ImGui::Spacing();
		constexpr auto web_link{ "github.com/garamond13/w-image-viewer" };
		ImGui::TextUnformatted(web_link);
		ImGui::Spacing();
		if (ImGui::Button("Copy link"))
			ImGui::SetClipboardText(web_link);
		ImGui::End();
	}
}

void User_interface::dialog_file_open()
{
	is_dialog_file_open = true;
	wiv_assert(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE), >= S_OK);
	Microsoft::WRL::ComPtr<IFileOpenDialog> file_open_dialog;
	if (CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(file_open_dialog.ReleaseAndGetAddressOf())) == S_OK) {
		constexpr std::array filterspec{
			COMDLG_FILTERSPEC{ L"All supported", WIV_SUPPORTED_FILE_TYPES }
		};
		wiv_assert(file_open_dialog->SetFileTypes(filterspec.size(), filterspec.data()), == S_OK);
		if (file_open_dialog->Show(hwnd) == S_OK) {
			Microsoft::WRL::ComPtr<IShellItem> shell_item;
			if (file_open_dialog->GetResult(shell_item.ReleaseAndGetAddressOf()) == S_OK) {
				wchar_t* path;
				if (shell_item->GetDisplayName(SIGDN_FILESYSPATH, &path) == S_OK) {
					if (file_manager.file_open(path))
						wiv_assert(PostMessageW(hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
					else
						wiv_assert(PostMessageW(hwnd, WIV_WM_RESET_RESOURCES, 0, 0), != 0);
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

void User_interface::reset_image_pan_and_zoom() noexcept
{
	image_pan = std::pair(0.0f, 0.0f);
	image_zoom = 1.0f;
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

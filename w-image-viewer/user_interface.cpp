#include "pch.h"
#include "user_interface.h"
#include "global.h"
#include "font.h"
#include "helpers.h"
#include "shader_config.h"
#include "version.h"
#include "supported_extensions.h"
#include "window.h"
#include "message.h"
#include "info.h"
#include "ensure.h"

enum WIV_OVERLAY_SHOW_ : uint64_t
{
    WIV_OVERLAY_SHOW_IMAGE_DIMS = 1 << 0,
    WIV_OVERLAY_SHOW_SCALE = 1 << 1,
    WIV_OVERLAY_SHOW_SCALED_DIMS = 1 << 2,
    WIV_OVERLAY_SHOW_KERNEL_INDEX = 1 << 3,
    WIV_OVERLAY_SHOW_KERNEL_SIZE = 1 << 4,
    WIV_OVERLAY_SHOW_KERNEL_RADIUS = 1 << 5,
    WIV_OVERLAY_SHOW_IMAGE_BITDEPTH = 1 << 6,
    WIV_OVERLAY_SHOW_IMAGE_NCHANNELS = 1 << 7,
    WIV_OVERLAY_SHOW_SCALE_FILTER = 1 << 8,
};

namespace
{
    // The order has to be same as in the enum WIV_KERNEL_FUNCTION_. 
    constexpr std::array kernel_function_names = {
        "Lanczos",
        "Ginseng",
        "Hamming",
        "Power of Cosine",
        "Kaiser",
        "Power of Garamond",
        "Power of Blackman",
        "GNW",
        "Said",
        "Nearest neighbor",
        "Linear",
        "Bicubic",
        "FSR kernel",
        "BC-Spline"
    };
}

User_interface::~User_interface()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void User_interface::create(ID3D11Device* device, ID3D11DeviceContext* device_context, bool* should_update)
{
    prenderer_should_update = should_update;

#ifndef NDEBUG
    IMGUI_CHECKVERSION();
#endif

    ImGui::CreateContext();

    // Font.
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesDefault());
    builder.AddChar(0x0161); // š
    ImVector<ImWchar> ranges;
    builder.BuildRanges(&ranges);
    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(ROBOTO_MEDIUM_COMPRESSED_DATA, ROBOTO_MEDIUM_COMPRESSED_SIZE, 16, nullptr, ranges.Data);
    ImGui::GetIO().Fonts->Build();
    
    // Set imgui.ini path.
    // ImGui does not store the path!
    static char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::strcpy(path, (std::filesystem::path(path).parent_path() / "imgui.ini").string().c_str());
    ImGui::GetIO().IniFilename = path;
    
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(device, device_context);
}

void User_interface::update()
{
    // Feed inputs to dear imgui, start new frame.
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    input();
    overlay();
    context_menu();
    window_settings();
    window_about();
    window_slideshow();

    // DEBUG ONLY!
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
    if (is_fullscreen || IsZoomed(g_hwnd)) {
        return;
    }
    RECT rect;
    
    // We need RECT::top if we are not centering window.
    if (!g_config.window_autowh_center.val) {
        GetWindowRect(g_hwnd, &rect);
    }
    else {
        rect.top = 0;
    }

    const double screen_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    const double screen_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    
    // If the image resolution is larger than the screen resolution * 0.9, downsize the window to screen resolution * 0.9 with the aspect ratio of the image.
    const double image_width = file_manager.image.get_width<double>();
    const double image_height = file_manager.image.get_height<double>();
    const double scale_factor = std::min(std::min(screen_width * 0.9 / image_width, (screen_height - rect.top) * 0.9 / image_height), 1.0);
    rect.right = std::lround(image_width * scale_factor);
    rect.bottom = std::lround(image_height * scale_factor);
    rect.left = 0;
    rect.top = 0;

    AdjustWindowRectEx(&rect, WIV_WINDOW_STYLE, FALSE, WIV_WINDOW_EX_STYLE);

    // Optionaly center the window and apply new dimensions.
    const UINT flags = static_cast<UINT>(g_config.window_autowh_center.val ? SWP_NOZORDER | SWP_NOCOPYBITS : SWP_NOZORDER | SWP_NOMOVE | SWP_NOCOPYBITS);
    SetWindowPos(g_hwnd, HWND_TOP, (screen_width - rc_w<double>(rect)) / 2.0, (screen_height - rc_h<double>(rect)) / 2.0, rc_w<int>(rect), rc_h<int>(rect), flags);
}

void User_interface::reset_image_panzoom() noexcept
{
    image_pan = ImVec2();
    image_zoom = 0.0f;
    image_no_scale = false;
}

void User_interface::slideshow()
{
    if (!is_slideshow_playing || file_manager.file_current.empty()) {
        
        // We need to reset these in case slide show gets started while we dont have a file.
        is_slideshow_playing = false;
        is_slideshow_start = false;

        return;
    }
    static std::chrono::high_resolution_clock::time_point start;
    
    // Upon slideshow start we need to set once duration for the first image (currently renderered image).
    if (is_slideshow_start) {
        start = std::chrono::high_resolution_clock::now();
        is_slideshow_start = false;
    }

    if (std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count() < slideshow_interval) {
        return;
    }
    file_manager.file_next();
    PostMessageW(g_hwnd, WIV_WM_OPEN_FILE, 0, 0);

    // This doesen't account for image load times.
    start = std::chrono::high_resolution_clock::now();
}

void User_interface::input()
{
    // Workaround for IsMouseDoubleClicked(0) triggering IsMouseDragging(0).
    static bool is_double_click;
    if (ImGui::IsMouseReleased(0))
        is_double_click = false;

    // Mouse
    //

    if (!ImGui::GetIO().WantCaptureMouse) {
        if (ImGui::IsMouseDoubleClicked(0)) {
            toggle_fullscreen();
            is_double_click = true;
            return;
        }

        // Image panning.
        if (!is_double_click && ImGui::IsMouseDragging(0)) {
            image_pan += ImGui::GetMouseDragDelta();
            ImGui::ResetMouseDragDelta();
            is_panning = true;
            *prenderer_should_update = true;
            return;
        }

        // Image zoom.
        if (ImGui::GetIO().MouseWheel > 0.0f) {
            image_zoom += 0.1f;
            is_zooming = true;
            *prenderer_should_update = true;
            return;
        }
        if (ImGui::GetIO().MouseWheel < 0.0f) {
            image_zoom -= 0.1f;
            is_zooming = true;
            *prenderer_should_update = true;
            return;
        }
    }

    //

    // Keyboard
    //

    if (!ImGui::GetIO().WantCaptureKeyboard) {

        // ctrl + key
        //

        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
            if (ImGui::IsKeyPressed(ImGuiKey_O, false)) {
                
                // It will block the thread, but probably no need to run it from new thread.
                dialog_file_open(WIV_OPEN_IMAGE);
                
                return;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_1, false)) {
                reset_image_panzoom();
                image_no_scale = true;
                *prenderer_should_update = true;
                return;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_0, false)) {
                reset_image_panzoom();
                *prenderer_should_update = true;
                return;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) {
                image_zoom += 0.1f;
                is_zooming = true;
                *prenderer_should_update = true;
                return;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) {
                image_zoom -= 0.1f;
                is_zooming = true;
                *prenderer_should_update = true;
                return;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                image_rotation -= 90.0f;
                is_rotating = true;
                *prenderer_should_update = true;
                return;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                image_rotation += 90.0f;
                is_rotating = true;
                *prenderer_should_update = true;
                return;
            }
        }

        //

        // alt + key
        //

        if (ImGui::IsKeyDown(ImGuiKey_LeftAlt)) {
            if (ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
                toggle_fullscreen();
                return;
            }
        }

        //

        // Free keys
        //

        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            if (file_manager.file_current.empty())
                return;
            file_manager.file_previous();
            ensure(PostMessageW(g_hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
            return;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            if (file_manager.file_current.empty())
                return;
            file_manager.file_next();
            ensure(PostMessageW(g_hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
            return;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
            toggle_fullscreen();
            return;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
            file_manager.delete_file();
            return;
        }

        //

    }

    // Leave escape key with higher priority.
    if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
        if (is_fullscreen)
            toggle_fullscreen();
        else
            ensure(DestroyWindow(g_hwnd), != 0);
        return;
    }

    //
}

void User_interface::overlay() const
{
    if (!is_overlay_open) {
        return;
    }

    // Set overlay position.
    const auto viewport = ImGui::GetMainViewport();
    constexpr float pad = 6.0f;
    ImVec2 window_pos;
    window_pos.x = g_config.overlay_position.val & 1 ? viewport->WorkPos.x + viewport->WorkSize.x - pad : viewport->WorkPos.x + pad;
    window_pos.y = g_config.overlay_position.val & 2 ? viewport->WorkPos.y + viewport->WorkSize.y - pad : viewport->WorkPos.y + pad;
    ImVec2 window_pos_pivot;
    window_pos_pivot.x = g_config.overlay_position.val & 1 ? 1.0f : 0.0f;
    window_pos_pivot.y = g_config.overlay_position.val & 2 ? 1.0f : 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("##overlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!g_config.overlay_config.val) {
            ImGui::TextUnformatted("Overlay is not configured.");
        }
        if (g_config.overlay_config.val & WIV_OVERLAY_SHOW_IMAGE_DIMS) {
            ImGui::Text("Image W: %i", info::image_width);
            ImGui::Text("Image H: %i", info::image_height);
        }
        if (g_config.overlay_config.val & WIV_OVERLAY_SHOW_IMAGE_BITDEPTH) {
            ImGui::Text("Image bitdepth: %i", info::image_bitdepth);
        }
        if (g_config.overlay_config.val & WIV_OVERLAY_SHOW_IMAGE_NCHANNELS) {
            ImGui::Text("Image nchannels: %i", info::image_nchannels);
        }
        if (g_config.overlay_config.val & WIV_OVERLAY_SHOW_SCALED_DIMS) {
            ImGui::Text("Scaled W: %i", info::scaled_width);
            ImGui::Text("Scaled H: %i", info::scaled_height);
        }
        if (g_config.overlay_config.val & WIV_OVERLAY_SHOW_SCALE) {
            ImGui::Text("Scale: %.6f", info::scale);
        }
        if (g_config.overlay_config.val & WIV_OVERLAY_SHOW_SCALE_FILTER) {
            ImGui::Text("Scale filter: %s", info::scale_filter.c_str());
        }
        if (g_config.overlay_config.val & WIV_OVERLAY_SHOW_KERNEL_INDEX) {
            ImGui::Text("Kernel: %s", kernel_function_names[info::kernel_index]);
        }
        if (g_config.overlay_config.val & WIV_OVERLAY_SHOW_KERNEL_INDEX) {
            ImGui::Text("Kernel radius: %.6f", info::kernel_radius);
        }
        if (g_config.overlay_config.val & WIV_OVERLAY_SHOW_KERNEL_SIZE) {
            ImGui::Text("Kernel size: %i", info::kernel_size);
        }
    }
    ImGui::End();
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
            ensure(PostMessageW(g_hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
            goto end;
        }
        if (ImGui::Selectable("Previous")) {
            if (file_manager.file_current.empty())
                goto end;
            file_manager.file_previous();
            ensure(PostMessageW(g_hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
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
            *prenderer_should_update = true;
            goto end;
        }
        if (ImGui::Selectable("Rotate CCW")) {
            image_rotation -= 90.0f;
            is_rotating = true;
            *prenderer_should_update = true;
            goto end;
        }
        ImGui::Separator();
        if (ImGui::Selectable("Delete")) {
            file_manager.delete_file();
            goto end;
        }
        ImGui::Separator();
        if (ImGui::Selectable("Overlay")) {
            is_overlay_open = !is_overlay_open;
            goto end;
        }
        ImGui::Separator();
        if (ImGui::Selectable("Slideshow...")) {
            is_window_slideshow_open = true;
            goto end;
        }
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
            ensure(DestroyWindow(g_hwnd), != 0);
            goto end;
        }
    end:
        ImGui::EndPopup();
    }
}

void User_interface::window_settings()
{
    // Early return.
    if (!is_window_settings_open)
        return;

    static constinit const ImVec2 window_size = { 430.0f, 696.0f };
    static constinit const ImVec2 button_size = { -1.0f, 0.0f };
    static int scale_profile_index; // The default profile should always be at index 0!
    ImGui::SetNextWindowSize(window_size, ImGuiCond_FirstUseEver);
    ImGui::Begin("Settings", &is_window_settings_open, ImGuiWindowFlags_NoCollapse);
    if (ImGui::CollapsingHeader("Window")) {
        ImGui::Spacing();
        ImGui::SeparatorText("Default dimensions");
        if (ImGui::Button("Use current dimensions##def")) {
            RECT rect;
            ensure(GetClientRect(g_hwnd, &rect), != 0);
            g_config.window_width.val = rect.right;
            g_config.window_height.val = rect.bottom;
        }
        ImGui::InputInt("Width##def", &g_config.window_width.val, 0, 0);
        ImGui::InputInt("Height##def", &g_config.window_height.val, 0, 0);
        ImGui::Spacing();
        ImGui::SeparatorText("Minimum dimensions");
        if (ImGui::Button("Use current dimensions##min")) {
            RECT rect;
            ensure(GetClientRect(g_hwnd, &rect), != 0);
            g_config.window_min_width.val = rect.right;
            g_config.window_min_height.val = rect.bottom;
        }
        ImGui::InputInt("Width##min", &g_config.window_min_width.val, 0, 0);
        ImGui::InputInt("Height##min", &g_config.window_min_height.val, 0, 0);
        ImGui::Spacing();
        ImGui::Checkbox("Keep aspect ratio when sizing", &g_config.window_keep_aspect.val);
        ImGui::Checkbox("Auto dimensions", &g_config.window_autowh.val);
        ImGui::BeginDisabled(!g_config.window_autowh.val);
        ImGui::Checkbox("Center on auto dimensions", &g_config.window_autowh_center.val);
        ImGui::EndDisabled();
        ImGui::Spacing();

        // The order has to be same as in the enum WIV_WINDOW_NAME_.
        static constinit const std::array window_name_items = {
            "Defualt name",
            "Filename",
            "Full filename"
        };

        ImGui::Combo("Title", &g_config.window_name.val, window_name_items.data(), window_name_items.size());
        ImGui::Spacing();

        ImGui::ColorEdit4("Background color", g_config.clear_color.val.data(), ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayHSV);
        ImGui::Spacing();
    }
    if (ImGui::CollapsingHeader("Scale")) {
        ImGui::Spacing();

        // Scale profiles
        //

        ImGui::SeparatorText("Profiles");
        static Right_open_range<float> range;
        std::array range_array = { &range.lower, &range.upper };
        ImGui::InputFloat2("##range", *range_array.data(), "%.6f");
        ImGui::SameLine();
        if (ImGui::Button("Add profile", button_size)) {
            if (range.is_valid()) {

                // Check first does profile already exists (overlaps with any other profile).
                bool exists = false;
                for (const auto& profile : g_config.scale_profiles) {
                    if (profile.range.is_overlapping(range)) {
                        exists = true;
                        break;
                    }
                }

                if (!exists) {
                    g_config.scale_profiles.push_back({ range, {} });
                    scale_profile_index = g_config.scale_profiles.size() - 1;
                }
                else
                    wiv_message(L"Overlapping profile. Profile overlaps with one of existing profiles.");
            }
            else
                wiv_message(L"Invalid range. Lower bound has to be lower or equal to upper bound.");
        }
        if (ImGui::Button("Edit profile", button_size)) {
            if (range.is_valid()) {

                // Check first is profile overlapping with any other profile.
                bool overlaps = false;
                for (int i{}; i < g_config.scale_profiles.size(); ++i)
                    if (i != scale_profile_index && g_config.scale_profiles[i].range.is_overlapping(range)) {
                        overlaps = true;
                        break;
                    }

                if (scale_profile_index > 0 && !overlaps) // We don't wanna allow edits on the default profile, nor overlaps with other profiles!
                    g_config.scale_profiles[scale_profile_index].range = range;

            }
            else
                wiv_message(L"Invalid range. Lower bound has to be lower or equal to upper bound.");
        }

        // Hold values.
        // Can't use vector<const char*> directly, because const char* will be non owning.
        std::vector<std::string> scale_profile_names;
        for (const auto& scale_profile_name : g_config.scale_profiles)
            scale_profile_names.push_back(std::to_string(scale_profile_name.range.lower) + ", " + std::to_string(scale_profile_name.range.upper));

        // Can't use std::string directly in ImGui.
        std::vector<const char*> profile_items;
        for (const auto& profile_item : scale_profile_names)
            profile_items.push_back(profile_item.c_str());


        ImGui::Combo("##profile", &scale_profile_index, profile_items.data(), profile_items.size());
        ImGui::SameLine();
        if (ImGui::Button("Remove profile", button_size)) {
            if (scale_profile_index > 0) { // Can't delete the default range!
                g_config.scale_profiles.erase(g_config.scale_profiles.begin() + scale_profile_index);
                --scale_profile_index;
            }
        }
        auto& scale = g_config.scale_profiles[scale_profile_index].config;
        ImGui::Spacing();

        //

        ImGui::SeparatorText("Pre-scale blur (downscale only)");
        ImGui::Checkbox("Enable pre-scale blur", &scale.blur_use.val);
        ImGui::BeginDisabled(!scale.blur_use.val);
        ImGui::InputInt("Radius##blur", &scale.blur_radius.val, 0, 0);
        scale.blur_radius.val = std::max(scale.blur_radius.val, 1);
        ImGui::InputFloat("Sigma##blur", &scale.blur_sigma.val, 0.0f, 0.0f, "%.6f");
        scale.blur_sigma.val = std::max(scale.blur_sigma.val, 1e-6f);
        ImGui::EndDisabled();
        ImGui::Spacing();
        ImGui::SeparatorText("Sigmoidize (upscale only)");
        ImGui::Checkbox("Enable sigmoidize", &scale.sigmoid_use.val);
        ImGui::BeginDisabled(!scale.sigmoid_use.val);
        ImGui::InputFloat("Contrast", &scale.sigmoid_contrast.val, 0.0f, 0.0f, "%.6f");
        scale.sigmoid_contrast.val = std::max(scale.sigmoid_contrast.val, 1e-6f);
        ImGui::InputFloat("Midpoint", &scale.sigmoid_midpoint.val, 0.0f, 0.0f, "%.6f");
        scale.sigmoid_midpoint.val = std::clamp(scale.sigmoid_midpoint.val, 0.0f, 1.0f);
        ImGui::EndDisabled();
        ImGui::Spacing();
        ImGui::SeparatorText("Filter");
        ImGui::Checkbox("Use cylindrical filtering (Jinc based)", &scale.kernel_cylindrical_use.val);
        ImGui::Combo("Kernel-function", &scale.kernel_index.val, kernel_function_names.data(), kernel_function_names.size());
        bool has_fixed_radius = false;
        switch (scale.kernel_index.val) {
            case WIV_KERNEL_FUNCTION_NEAREST:
            case WIV_KERNEL_FUNCTION_LINEAR:
            case WIV_KERNEL_FUNCTION_BICUBIC:
            case WIV_KERNEL_FUNCTION_FSR:
            case WIV_KERNEL_FUNCTION_BCSPLINE:
                has_fixed_radius = true;
        }
        ImGui::BeginDisabled(has_fixed_radius);
        ImGui::InputFloat("Radius##kernel", &scale.kernel_radius.val, 0.0f, 0.0f, "%.6f");
        ImGui::InputFloat("Blur", &scale.kernel_blur.val, 0.0f, 0.0f, "%.6f");
        ImGui::EndDisabled();
        bool hasno_parameter1 = false;
        switch (scale.kernel_index.val) {
            case WIV_KERNEL_FUNCTION_LANCZOS:
            case WIV_KERNEL_FUNCTION_GINSENG:
            case WIV_KERNEL_FUNCTION_HAMMING:
            case WIV_KERNEL_FUNCTION_NEAREST:
            case WIV_KERNEL_FUNCTION_LINEAR:
            case WIV_KERNEL_FUNCTION_FSR:
                hasno_parameter1 = true;
        }
        ImGui::BeginDisabled(hasno_parameter1);
        ImGui::InputFloat("Parameter 1", &scale.kernel_parameter1.val, 0.0f, 0.0f, "%.6f");
        ImGui::EndDisabled();
        bool hasno_parameter2 = false;
        switch (scale.kernel_index.val) {
            case WIV_KERNEL_FUNCTION_LANCZOS:
            case WIV_KERNEL_FUNCTION_GINSENG:
            case WIV_KERNEL_FUNCTION_HAMMING:
            case WIV_KERNEL_FUNCTION_POW_COSINE:
            case WIV_KERNEL_FUNCTION_KAISER:
            case WIV_KERNEL_FUNCTION_NEAREST:
            case WIV_KERNEL_FUNCTION_LINEAR:
            case WIV_KERNEL_FUNCTION_BICUBIC:
            case WIV_KERNEL_FUNCTION_FSR:
                hasno_parameter2 = true;
        }
        ImGui::BeginDisabled(hasno_parameter2);
        ImGui::InputFloat("Parameter 2", &scale.kernel_parameter2.val, 0.0f, 0.0f, "%.6f");
        ImGui::EndDisabled();
        ImGui::InputFloat("Anti-ringing", &scale.kernel_antiringing.val, 0.0f, 0.0f, "%.6f");
        scale.kernel_antiringing.val = std::clamp(scale.kernel_antiringing.val, 0.0f, 1.0f);
        ImGui::Spacing();
        ImGui::SeparatorText("Post-scale unsharp mask");
        ImGui::Checkbox("Enable post-scale unsharp mask", &scale.unsharp_use.val);
        ImGui::BeginDisabled(!scale.unsharp_use.val);
        ImGui::InputInt("Radius##unsharp", &scale.unsharp_radius.val, 0, 0);
        scale.unsharp_radius.val = std::max(scale.unsharp_radius.val, 1);
        ImGui::InputFloat("Sigma##unsharp", &scale.unsharp_sigma.val, 0.0f, 0.0f, "%.6f");
        scale.unsharp_sigma.val = std::max(scale.unsharp_sigma.val, 1e-6f);
        ImGui::InputFloat("Amount", &scale.unsharp_amount.val, 0.0f, 0.0f, "%.6f");
        scale.unsharp_amount.val = std::max(scale.unsharp_amount.val, 1e-6f);
        ImGui::EndDisabled();
        ImGui::Spacing();
    }
    if (ImGui::CollapsingHeader("Color management")) {
        ImGui::Spacing();
        ImGui::Checkbox("Enable color management", &g_config.cms_use.val);
        ImGui::Spacing();
        ImGui::BeginDisabled(!g_config.cms_use.val);

        // The order has to be the same as in the enum WIV_CMS_PROFILE_DISPLAY_.
        static constinit const std::array cms_display_profile_items = {
            "Auto",
            "sRGB",
            "AdobeRGB",
            "Custom"
        };

        ImGui::Combo("Display profile", &g_config.cms_display_profile.val, cms_display_profile_items.data(), cms_display_profile_items.size());
        char buffer[MAX_PATH];
        std::strcpy(buffer, g_config.cms_display_profile_custom.val.string().c_str());
        ImGui::BeginDisabled(g_config.cms_display_profile.val != WIV_CMS_PROFILE_DISPLAY_CUSTOM);
        ImGui::InputText("##custom_path", buffer, MAX_PATH);
        g_config.cms_display_profile_custom.val = buffer;
        ImGui::SameLine();
        if (ImGui::Button("Custom...", button_size)) {
            std::thread t(&User_interface::dialog_file_open, this, WIV_OPEN_ICC);
            t.detach();
        }
        ImGui::EndDisabled();
        ImGui::Spacing();

        // The order has to be the same as the lcms2 ICC Intents.
        static constinit const std::array cms_intent_items = {
            "Perceptual",
            "Relative colorimetric",
            "Saturation",
            "Absolute colorimetric"
        };

        ImGui::Combo("Rendering intent", &g_config.cms_intent.val, cms_intent_items.data(), cms_intent_items.size());
        ImGui::Checkbox("Black Point Compensation", &g_config.cms_bpc_use.val);

        // LUT size
        static constinit const std::array cms_lut_size_items = {
            "33",
            "49",
            "65",
        };
        int cms_lut_size_items_index;
        switch (g_config.cms_lut_size.val) {
            case 33:
                cms_lut_size_items_index = 0;
                break;
            case 49:
                cms_lut_size_items_index = 1;
                break;
            case 65:
                cms_lut_size_items_index = 2;
        }
        ImGui::Combo("LUT size", &cms_lut_size_items_index, cms_lut_size_items.data(), cms_lut_size_items.size());
        switch (cms_lut_size_items_index) {
            case 0:
                g_config.cms_lut_size.val = 33;
                break;
            case 1:
                g_config.cms_lut_size.val = 49;
                break;
            case 2:
                g_config.cms_lut_size.val = 65;
        }
        
        ImGui::Checkbox("Dither 8 bit images", &g_config.cms_dither.val);
        ImGui::EndDisabled();
        ImGui::Spacing();
        ImGui::SeparatorText("Color tags");
        ImGui::Checkbox("Linear tagged images default to ACEScg", &g_config.cms_default_to_aces.val);
        ImGui::Checkbox("Untagged images default to sRGB", &g_config.cms_default_to_srgb.val);
        ImGui::Spacing();
    }
    if (ImGui::CollapsingHeader("Transparency")) {
        ImGui::Spacing();
        ImGui::InputFloat("Tile size", &g_config.alpha_tile_size.val, 0.0f, 0.0f, "%.6f");
        ImGui::ColorEdit3("First tile color", g_config.alpha_tile1_color.val.data(), ImGuiColorEditFlags_DisplayHSV);
        ImGui::ColorEdit3("Second tile color", g_config.alpha_tile2_color.val.data(), ImGuiColorEditFlags_DisplayHSV);
        ImGui::Spacing();
    }
    if (ImGui::CollapsingHeader("Overlay")) {
        ImGui::Spacing();
        ImGui::Checkbox("Show overlay on start", &g_config.overlay_show.val);
        static constinit const std::array overlay_position_items = {
            "Top-left",
            "Top-right",
            "Bottom-left",
            "Bottom-right"
        };
        ImGui::Combo("Overlay position", &g_config.overlay_position.val, overlay_position_items.data(), overlay_position_items.size());
        ImGui::Spacing();
        ImGui::TextUnformatted("Show:");
        if (ImGui::Selectable("Image dimensions", g_config.overlay_config.val & WIV_OVERLAY_SHOW_IMAGE_DIMS)) {
            g_config.overlay_config.val ^= WIV_OVERLAY_SHOW_IMAGE_DIMS;
        }
        if (ImGui::Selectable("Image bit depth", g_config.overlay_config.val & WIV_OVERLAY_SHOW_IMAGE_BITDEPTH)) {
            g_config.overlay_config.val ^= WIV_OVERLAY_SHOW_IMAGE_BITDEPTH;
        }
        if (ImGui::Selectable("Image nchannels", g_config.overlay_config.val & WIV_OVERLAY_SHOW_IMAGE_NCHANNELS)) {
            g_config.overlay_config.val ^= WIV_OVERLAY_SHOW_IMAGE_NCHANNELS;
        }
        if (ImGui::Selectable("Scaled dimensions", g_config.overlay_config.val & WIV_OVERLAY_SHOW_SCALED_DIMS)) {
            g_config.overlay_config.val ^= WIV_OVERLAY_SHOW_SCALED_DIMS;
        }
        if (ImGui::Selectable("Scale factor", g_config.overlay_config.val & WIV_OVERLAY_SHOW_SCALE)) {
            g_config.overlay_config.val ^= WIV_OVERLAY_SHOW_SCALE;
        }
        if (ImGui::Selectable("Scale filter", g_config.overlay_config.val & WIV_OVERLAY_SHOW_SCALE_FILTER)) {
            g_config.overlay_config.val ^= WIV_OVERLAY_SHOW_SCALE_FILTER;
        }
        if (ImGui::Selectable("Kernel function", g_config.overlay_config.val & WIV_OVERLAY_SHOW_KERNEL_INDEX)) {
            g_config.overlay_config.val ^= WIV_OVERLAY_SHOW_KERNEL_INDEX;
        }
        if (ImGui::Selectable("Scale kernel radius", g_config.overlay_config.val & WIV_OVERLAY_SHOW_KERNEL_RADIUS)) {
            g_config.overlay_config.val ^= WIV_OVERLAY_SHOW_KERNEL_RADIUS;
        }
        if (ImGui::Selectable("Scale kernel size", g_config.overlay_config.val & WIV_OVERLAY_SHOW_KERNEL_SIZE)) {
            g_config.overlay_config.val ^= WIV_OVERLAY_SHOW_KERNEL_SIZE;
        }
        ImGui::Spacing();
    }
    if (ImGui::CollapsingHeader("Other")) {
        ImGui::Spacing();

        // The order has to be same as in WIV_PASS_FORMATS array.
        static constinit const std::array internal_format_items = {
            "RGBA16",
            "RGBA32F"
        };

        ImGui::Combo("Internal format", &g_config.pass_format.val, internal_format_items.data(), internal_format_items.size());
        ImGui::Spacing();
        ImGui::Checkbox("Read only thumbnails in RAW images", &g_config.raw_thumb.val);
        ImGui::Spacing();
        ImGui::Checkbox("Cycle files on Next/Previous", &g_config.cycle_files.val);
        ImGui::Spacing();
    }
    ImGui::SeparatorText("Changes");
    if (ImGui::Button("Revert changes", button_size)) {
        scale_profile_index = 0;
        g_config.read();
    }
    if (ImGui::Button("Write changes", button_size))
        g_config.write();
    ImGui::Spacing();
    ImGui::End();
}

void User_interface::window_slideshow()
{
    if (!is_window_slideshow_open)
        return;
    static constinit const ImVec2 size = { 288.0f, 0.0f };
    ImGui::SetNextWindowSize(size);
    ImGui::Begin("Slideshow", &is_window_slideshow_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    static bool auto_close;
    ImGui::Checkbox("Auto close this window on start", &auto_close);
    ImGui::InputFloat("Interval", &slideshow_interval, 0.0f, 0.0f, "%.3f");
    if (ImGui::Button("Start")) {
        if (auto_close) {
            is_window_slideshow_open = false;
        }
        is_slideshow_start = true;
        is_slideshow_playing = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        is_slideshow_playing = false;
    }
    ImGui::End();
}

void User_interface::window_about()
{
    // Early return.
    if (!is_window_about_open)
        return;

    static constinit const ImVec2 size = { 288, 178.0f };
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

void User_interface::dialog_file_open(WIV_OPEN_ file_type)
{
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
        return;
    }
    is_dialog_file_open = true;
    Microsoft::WRL::ComPtr<IFileOpenDialog> file_open_dialog;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(file_open_dialog.GetAddressOf())))) {
        COMDLG_FILTERSPEC filterspec = {};
        filterspec.pszName = L"All supported";
        filterspec.pszSpec = file_type == WIV_OPEN_IMAGE ? WIV_SUPPORTED_EXTENSIONS : L"*.icc" /* WIV_OPEN_ICC */;
        ensure(file_open_dialog->SetFileTypes(1, &filterspec), == S_OK);
        if (SUCCEEDED(file_open_dialog->Show(g_hwnd))) {
            Microsoft::WRL::ComPtr<IShellItem> shell_item;
            if (SUCCEEDED(file_open_dialog->GetResult(shell_item.GetAddressOf()))) {
                wchar_t* path;
                if (SUCCEEDED(shell_item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                    if (file_type == WIV_OPEN_IMAGE) {
                        if (file_manager.file_open(path)) {
                            ensure(PostMessageW(g_hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
                        }
                        else {
                            ensure(PostMessageW(g_hwnd, WIV_WM_RESET_RESOURCES, 0, 0), != 0);
                        }
                    }

                    // WIV_OPEN_ICC
                    else {
                        g_config.cms_display_profile_custom.val = path;
                    }
                    
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
    static WINDOWPLACEMENT windowplacment;
    if (is_fullscreen) {
        SetWindowLongPtrW(g_hwnd, GWL_STYLE, WIV_WINDOW_STYLE);
        ensure(SetWindowPlacement(g_hwnd, &windowplacment), != 0);
        ensure(SetWindowPos(g_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOCOPYBITS), != 0);
    }
    else {
        ensure(GetWindowPlacement(g_hwnd, &windowplacment), != 0);
        SetWindowLongPtrW(g_hwnd, GWL_STYLE, WS_POPUP);
        const int cx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        const int cy = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        ensure(SetWindowPos(g_hwnd, HWND_TOP, 0, 0, cx, cy, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOCOPYBITS), != 0);
    }
    is_fullscreen = !is_fullscreen;
}
#include "pch.h"
#include "global.h"
#include "window.h"
#include "helpers.h"
#include "resource.h"

enum WIV_WINDOW_NAME_
{
    WIV_WINDOW_NAME_DEFAULT,
    WIV_WINDOW_NAME_FILE_NAME,
    WIV_WINDOW_NAME_FILE_NAME_FULL
};

namespace {
    constexpr auto WIV_WINDOW_NAME{ L"W Image Viewer" };
}

// Forward declare message handler from imgui_impl_win32.cpp.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Window::Window(HINSTANCE hinstance, int ncmdshow)
{
    // Register window class.
    const WNDCLASSEXW wndclassexw{
        .cbSize{ sizeof(WNDCLASSEXW) },
        .style{ CS_HREDRAW | CS_VREDRAW },
        .lpfnWndProc{ wnd_proc },
        .hInstance{ hinstance },
        .hIcon{ LoadIconW(hinstance, MAKEINTRESOURCEW(IDI_WIMAGEVIEWER)) },
        .lpszClassName{ L"WIV" }
    };
    wiv_assert(RegisterClassExW(&wndclassexw), != 0);

    // Create window.
    RECT rect{
        .right{ g_config.window_width.val },
        .bottom{ g_config.window_height.val }
    };
    wiv_assert(AdjustWindowRectEx(&rect, WIV_WINDOW_STYLE, FALSE, WIV_WINDOW_EX_STYLE), != 0);
    CreateWindowExW(WIV_WINDOW_EX_STYLE, wndclassexw.lpszClassName, WIV_WINDOW_NAME, WIV_WINDOW_STYLE, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hinstance, this);
    renderer.create(hwnd);
    ShowWindow(hwnd, ncmdshow);
}

void Window::set_window_name() const
{
    std::wstring name;
    switch (g_config.window_name.val) {
        case WIV_WINDOW_NAME_DEFAULT:
            name = WIV_WINDOW_NAME;
            break;
        case WIV_WINDOW_NAME_FILE_NAME:
            name = renderer.user_interface.file_manager.file_current.filename().wstring() + L" - " + WIV_WINDOW_NAME;
            break;
        case WIV_WINDOW_NAME_FILE_NAME_FULL:
            name = renderer.user_interface.file_manager.file_current.wstring() + L" - " + WIV_WINDOW_NAME;
            break;
    }
    wiv_assert(SetWindowTextW(hwnd, name.c_str()), != 0);
}

void Window::reset_image_rotation() noexcept
{
    // Rotated 180.
    if (renderer.user_interface.file_manager.image.orientation == 2)
        renderer.user_interface.image_rotation = -180.0f;

    // Rotated 90 cw.
    else if (renderer.user_interface.file_manager.image.orientation == 5)
        renderer.user_interface.image_rotation = -90.0f;

    // Rotated 90 ccw.
    else if (renderer.user_interface.file_manager.image.orientation == 7)
        renderer.user_interface.image_rotation = 90.0f;

    else
        renderer.user_interface.image_rotation = 0.0f;

    // Reset orientation.
    renderer.user_interface.file_manager.image.orientation = 0;
}

LRESULT Window::wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
        return 1;

    // Get "this" pointer that we passed to CreateWindowExW().
    auto* window{ reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)) };

    switch (message) {
        
        // WM_NCCREATE is not guarantied to be the first message.
        [[unlikely]] case WM_NCCREATE:
            window = reinterpret_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);
            window->hwnd = hwnd;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
            return DefWindowProcW(hwnd, message, wparam, lparam);

        case WM_ERASEBKGND:
            return 1;

            // Set minimum window size.
        case WM_GETMINMAXINFO:
            reinterpret_cast<MINMAXINFO*>(lparam)->ptMinTrackSize.x = GetSystemMetrics(SM_CXMIN);
            reinterpret_cast<MINMAXINFO*>(lparam)->ptMinTrackSize.y = GetSystemMetrics(SM_CYMIN) + 1;
            break;

            // Provides the mosuse cursor in fullscren on mousemove.
            // Without this, after exiting from fullscreen the mosue cursor may stay hidden.
        case WM_MOUSEMOVE:
            while (ShowCursor(TRUE) < 0);
            break;
        case WM_NCMOUSEMOVE:
            while (ShowCursor(TRUE) < 0);
            break;

        case WIV_WM_OPEN_FILE:
            window->renderer.create_image();
            if(g_config.window_autowh.val)
                window->renderer.user_interface.auto_window_size();
            window->renderer.user_interface.reset_image_panzoom();
            window->reset_image_rotation();
            window->set_window_name();
            window->renderer.should_update = true;
            break;
        case WM_SIZE:
            if (wparam != SIZE_MINIMIZED) {
                window->renderer.on_window_resize();
                window->renderer.should_update = true;
            }
            break;
        case WM_DROPFILES:
            if (window->renderer.user_interface.file_manager.drag_and_drop(reinterpret_cast<HDROP>(wparam))) {
                window->renderer.create_image();
                if (g_config.window_autowh.val)
                    window->renderer.user_interface.auto_window_size();
                window->renderer.user_interface.reset_image_panzoom();
                window->reset_image_rotation();
                window->set_window_name();
                window->renderer.should_update = true;
            }
            break;
        case WIV_WM_RESET_RESOURCES:
            window->renderer.reset_resources();
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(hwnd, message, wparam, lparam);
    }
    return 0;
}

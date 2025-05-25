#include "pch.h"
#include "include\global.h"
#include "window.h"
#include "include\helpers.h"
#include "resources\Resource.h"
#include "include\ensure.h"

enum WIV_WINDOW_NAME_
{
    WIV_WINDOW_NAME_DEFAULT,
    WIV_WINDOW_NAME_FILE_NAME,
    WIV_WINDOW_NAME_FILE_NAME_FULL
};

namespace
{
    constexpr auto WIV_WINDOW_NAME = L"W Image Viewer";
}

Window::Window(HINSTANCE hinstance, int ncmdshow)
{
    // Register window class.
    WNDCLASSEXW wndclassexw = {};
    wndclassexw.cbSize = sizeof(WNDCLASSEXW);
    wndclassexw.style = CS_HREDRAW | CS_VREDRAW;
    wndclassexw.lpfnWndProc = wndproc;
    wndclassexw.hInstance = hinstance;
    wndclassexw.hIcon = LoadIconW(hinstance, MAKEINTRESOURCEW(IDI_WIMAGEVIEWER));
    wndclassexw.lpszClassName = L"WIV";
    ensure(RegisterClassExW(&wndclassexw), != 0);

    // Create window.
    RECT rect = {};
    rect.right = g_config.window_width.val;
    rect.bottom = g_config.window_height.val;
    ensure(AdjustWindowRectEx(&rect, WIV_WINDOW_STYLE, FALSE, WIV_WINDOW_EX_STYLE), != 0);
    g_hwnd = CreateWindowExW(WIV_WINDOW_EX_STYLE, wndclassexw.lpszClassName, WIV_WINDOW_NAME, WIV_WINDOW_STYLE, CW_USEDEFAULT, CW_USEDEFAULT, rc_w<int>(rect), rc_h<int>(rect), nullptr, nullptr, hinstance, this);
    assert(g_hwnd);

    renderer.init();
    ShowWindow(g_hwnd, ncmdshow);
}

int Window::message_loop()
{
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        else {
            if (!is_minimized) {
                renderer.ui.slideshow();
                renderer.update();
                renderer.draw();
                renderer.fullscreen_hide_cursor();
            }
            else {
                WaitMessage();
            }
        }
    }
    return msg.wParam;
}

// Forward declare message handler from imgui_impl_win32.cpp.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* static */ LRESULT Window::wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam)) {
        return 1;
    }
    Window* window = nullptr;
    [[unlikely]] if (message == WM_NCCREATE) {
        auto* createstruct = reinterpret_cast<CREATESTRUCTW*>(lparam);
        window = reinterpret_cast<Window*>(createstruct->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        g_hwnd = hwnd;
    }
    else {
        window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (window) {
        return window->handle_message(hwnd, message, wparam, lparam);
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}    

LRESULT Window::handle_message(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {
        case WM_ERASEBKGND:
            return 1;

        // Set minimum window size.
        case WM_GETMINMAXINFO:
        {
            RECT rect = {};
            rect.right = g_config.window_min_width.val;
            rect.bottom = g_config.window_min_height.val;
            ensure(AdjustWindowRectEx(&rect, WIV_WINDOW_STYLE, FALSE, WIV_WINDOW_EX_STYLE), != 0);
            auto minmaxinfo = reinterpret_cast<MINMAXINFO*>(lparam);
            minmaxinfo->ptMinTrackSize.x = std::max(static_cast<LONG>(GetSystemMetrics(SM_CXMIN)), rc_w<LONG>(rect));
            minmaxinfo->ptMinTrackSize.y = std::max(static_cast<LONG>(GetSystemMetrics(SM_CYMIN) + 1), rc_h<LONG>(rect));
            return 0;
        }

        // Provides the mosuse cursor in fullscren on mousemove.
        // Without this, after exiting from fullscreen the mosue cursor may stay hidden.
        case WM_MOUSEMOVE:
            while (ShowCursor(TRUE) < 0);
            return 0;
        case WM_NCMOUSEMOVE:
            while (ShowCursor(TRUE) < 0);
            return 0;

        case WIV_WM_OPEN_FILE:
            renderer.create_image();
            if (g_config.window_autowh.val) {
                renderer.ui.auto_window_size();
            }
            renderer.ui.reset_image_panzoom();
            reset_image_rotation();
            set_window_name();
            renderer.should_update = true;
            return 0;
        case WM_SIZING:
            if (g_config.window_keep_aspect.val && renderer.ui.file_manager.image.is_valid()) {
                auto rect = reinterpret_cast<RECT*>(lparam);

                // Get client area.
                RECT unadjusted_rect = *rect;
                ensure(UnAdjustWindowRectEx(&unadjusted_rect, WIV_WINDOW_STYLE, FALSE, WIV_WINDOW_EX_STYLE), != 0);
                const auto client_width = rc_w<double>(unadjusted_rect);
                const auto client_height = rc_h<double>(unadjusted_rect);

                const auto image_aspect = renderer.ui.file_manager.image.get_aspect<double>();
                const auto new_client_width = std::lround(client_height * image_aspect - client_width);
                const auto new_client_height = std::lround(client_width / image_aspect - client_height);
                switch (wparam) {
                    case WMSZ_LEFT:
                    case WMSZ_RIGHT:
                    case WMSZ_BOTTOMLEFT:
                    case WMSZ_BOTTOMRIGHT:
                        rect->bottom += new_client_height;
                        break;
                    case WMSZ_TOP:
                    case WMSZ_BOTTOM:
                        rect->right += new_client_width;
                        break;
                    case WMSZ_TOPLEFT:
                    case WMSZ_TOPRIGHT:
                        rect->top -= new_client_height;
                }
                return 1;
            }
            break;
        case WM_SIZE:
            is_minimized = wparam == SIZE_MINIMIZED;
            if (!is_minimized) {
                renderer.on_window_resize();
                renderer.should_update = true;
            }
            return 0;
        case WM_DROPFILES:
            ensure(SetForegroundWindow(hwnd), != 0);
            if (renderer.ui.file_manager.drag_and_drop(reinterpret_cast<HDROP>(wparam))) {
                renderer.create_image();
                if (g_config.window_autowh.val) {
                    renderer.ui.auto_window_size();
                }
                renderer.ui.reset_image_panzoom();
                reset_image_rotation();
                set_window_name();
                renderer.should_update = true;
            }
            return 0;
        case WIV_WM_RESET_RESOURCES:
            renderer.reset_resources();
            ensure(SetWindowTextW(hwnd, WIV_WINDOW_NAME), != 0);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

void Window::set_window_name() const
{
    std::wstring name;
    switch (g_config.window_name.val) {
        case WIV_WINDOW_NAME_DEFAULT:
            name = WIV_WINDOW_NAME;
            break;
        case WIV_WINDOW_NAME_FILE_NAME:
            name = renderer.ui.file_manager.file_current.filename().wstring() + L" - " + WIV_WINDOW_NAME;
            break;
        case WIV_WINDOW_NAME_FILE_NAME_FULL:
            name = renderer.ui.file_manager.file_current.wstring() + L" - " + WIV_WINDOW_NAME;
            break;
    }
    ensure(SetWindowTextW(g_hwnd, name.c_str()), != 0);
}

void Window::reset_image_rotation() noexcept
{
    // Rotated 180.
    if (renderer.ui.file_manager.image.orientation == 2) {
        renderer.ui.image_rotation = -180;
    }

    // Rotated 90 cw.
    else if (renderer.ui.file_manager.image.orientation == 5) {
        renderer.ui.image_rotation = -90;
    }

    // Rotated 90 ccw.
    else if (renderer.ui.file_manager.image.orientation == 7) {
        renderer.ui.image_rotation = 90;
    }

    else {
        renderer.ui.image_rotation = 0;
    }

    // Reset orientation.
    renderer.ui.file_manager.image.orientation = 0;
}

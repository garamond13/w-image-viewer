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
        .lpfnWndProc{ wndproc },
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
    g_hwnd = CreateWindowExW(WIV_WINDOW_EX_STYLE, wndclassexw.lpszClassName, WIV_WINDOW_NAME, WIV_WINDOW_STYLE, CW_USEDEFAULT, CW_USEDEFAULT, rc_w<int>(rect), rc_h<int>(rect), nullptr, nullptr, hinstance, this);

    renderer.create();
    ShowWindow(g_hwnd, ncmdshow);
}

int Window::message_loop()
{
    MSG msg{};
    while (msg.message != WM_QUIT) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        else {
            if (!is_minimized) {
                renderer.update();
                renderer.draw();
                renderer.fullscreen_hide_cursor();
            }
            else
                WaitMessage();
        }
    }
    return static_cast<int>(msg.wParam);
}

/* static */ LRESULT Window::wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
        return 1;

    // Get "this" pointer that we passed to CreateWindowExW().
    auto* window{ reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)) };

    switch (message) {

        // WM_NCCREATE is not guarantied to be the first message.
        [[unlikely]] case WM_NCCREATE:
            window = reinterpret_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);
            g_hwnd = hwnd;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
            break;

        case WM_ERASEBKGND:
            return 1;

        // Set minimum window size.
        case WM_GETMINMAXINFO: {
            RECT rect{
                .right{ g_config.window_min_width.val },
                .bottom{ g_config.window_min_height.val }
            };
            wiv_assert(AdjustWindowRectEx(&rect, WIV_WINDOW_STYLE, FALSE, WIV_WINDOW_EX_STYLE), != 0);
            auto minmaxinfo{ reinterpret_cast<MINMAXINFO*>(lparam) };
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
            window->renderer.create_image();
            if (g_config.window_autowh.val)
                window->renderer.ui.auto_window_size();
            window->renderer.ui.reset_image_panzoom();
            window->reset_image_rotation();
            window->set_window_name();
            window->renderer.should_update = true;
            return 0;
        case WM_SIZING:
            if (g_config.window_keep_aspect.val && window->renderer.ui.file_manager.image.isn_null()) {
                auto rect{ reinterpret_cast<RECT*>(lparam) };

                // Get client area.
                RECT unadjusted_rect{ *rect };
                wiv_assert(UnAdjustWindowRectEx(&unadjusted_rect, WIV_WINDOW_STYLE, FALSE, WIV_WINDOW_EX_STYLE), != 0);
                const auto client_width{ rc_w<double>(unadjusted_rect) };
                const auto client_height{ rc_h<double>(unadjusted_rect) };

                const auto image_aspect{ window->renderer.ui.file_manager.image.get_aspect<double>() };
                const auto new_client_width{ std::lround(client_height * image_aspect - client_width)};
                const auto new_client_height{ std::lround(client_width / image_aspect - client_height) };
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
            window->is_minimized = wparam == SIZE_MINIMIZED;
            if (!window->is_minimized) {
                window->renderer.on_window_resize();
                window->renderer.should_update = true;
            }
            return 0;
        case WM_DROPFILES:
            wiv_assert(SetForegroundWindow(hwnd), != 0);
            if (window->renderer.ui.file_manager.drag_and_drop(reinterpret_cast<HDROP>(wparam))) {
                window->renderer.create_image();
                if (g_config.window_autowh.val)
                    window->renderer.ui.auto_window_size();
                window->renderer.ui.reset_image_panzoom();
                window->reset_image_rotation();
                window->set_window_name();
                window->renderer.should_update = true;
            }
            return 0;
        case WIV_WM_RESET_RESOURCES:
            window->renderer.reset_resources();
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
    wiv_assert(SetWindowTextW(g_hwnd, name.c_str()), != 0);
}

void Window::reset_image_rotation() noexcept
{
    // Rotated 180.
    if (renderer.ui.file_manager.image.orientation == 2)
        renderer.ui.image_rotation = -180.0f;

    // Rotated 90 cw.
    else if (renderer.ui.file_manager.image.orientation == 5)
        renderer.ui.image_rotation = -90.0f;

    // Rotated 90 ccw.
    else if (renderer.ui.file_manager.image.orientation == 7)
        renderer.ui.image_rotation = 90.0f;

    else
        renderer.ui.image_rotation = 0.0f;

    // Reset orientation.
    renderer.ui.file_manager.image.orientation = 0;
}

#include "pch.h"
#include "window.h"
#include "helpers.h"
#include "resource.h"

Window::Window(Config* p_config, HINSTANCE hinstance, int ncmdshow) :
	p_config(p_config)
{
    //register window class
    WNDCLASSEXW wndclassexw{
        .cbSize{ sizeof(WNDCLASSEXW) },
        .style{ CS_HREDRAW | CS_VREDRAW },
        .lpfnWndProc{ wnd_proc },
        .hInstance{ hinstance },
        .hIcon{ LoadIconW(hinstance, MAKEINTRESOURCEW(IDI_WIMAGEVIEWER)) },
        .lpszClassName{ L"wiv" }
    };
    wiv_assert(RegisterClassExW(&wndclassexw), != 0);

    //create window
    RECT rect{
        .right{ p_config->window_w },
        .bottom{ p_config->window_h }
    };
    wiv_assert(AdjustWindowRectEx(&rect, WIV_WINDOW_STYLE, FALSE, WIV_WINDOW_EX_STYLE), != 0);
    CreateWindowExW(WIV_WINDOW_EX_STYLE, wndclassexw.lpszClassName, L"W Image Viewer", WIV_WINDOW_STYLE, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hinstance, this);
    renderer.create(p_config, hwnd);
    ShowWindow(hwnd, ncmdshow);
}

//forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
        return 1;

    //get "this pointer"
    auto* window{ reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)) };

    switch (message) {
        //WM_NCCREATE is not guarantied to be the first message
    [[unlikely]] case WM_NCCREATE:
        window = reinterpret_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);
        window->hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        return DefWindowProcW(hwnd, message, wparam, lparam);

    case WM_ERASEBKGND:
        return 1;

        //set minimum window size
    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO*>(lparam)->ptMinTrackSize.x = GetSystemMetrics(SM_CXMIN);
        reinterpret_cast<MINMAXINFO*>(lparam)->ptMinTrackSize.y = GetSystemMetrics(SM_CYMIN) + 1;
        break;

        //provides the mosuse cursor in fullscren on mousemove
        //without this, after exiting from fullscreen the mosue cursor may stay hidden
    case WM_MOUSEMOVE:
        while (ShowCursor(TRUE) < 0);
        break;
    case WM_NCMOUSEMOVE:
        while (ShowCursor(TRUE) < 0);
        break;

    case WIV_WM_OPEN_FILE:
        window->renderer.create_image();
        if(window->p_config->window_autowh)
            window->renderer.user_interface.auto_window_size();
        window->renderer.should_update = true;
        break;
    case WM_SIZE:
        window->renderer.on_window_resize();
        window->renderer.should_update = true;
        break;
    case WM_DROPFILES:
        if (window->renderer.user_interface.file_manager.drag_and_drop(reinterpret_cast<HDROP>(wparam))) {
            window->renderer.create_image();
            if (window->p_config->window_autowh)
                window->renderer.user_interface.auto_window_size();
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

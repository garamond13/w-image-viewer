#include "pch.h"
#include "config.h"
#include "window.h"
#include "helpers.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    auto config{ std::make_unique<Config>() };
    config->file_read();
    config->values_map();
    auto window{ std::make_unique<Window>(config.get(), hInstance, nCmdShow) };

    //"direct" file open
    if (lpCmdLine[0] != '\0') {
        [[maybe_unused]] int argc;
        auto argv{ CommandLineToArgvW(lpCmdLine, &argc) };
        window->renderer.user_interface.file_manager.file_open(argv[0]);
        wiv_assert(PostMessageW(window->hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
        LocalFree(argv);
    }

    //message loop
    MSG msg{};
    while (msg.message != WM_QUIT) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        else {
            window->renderer.update();
            window->renderer.draw();
            window->renderer.fullscreen_hide_cursor();
        }
    }

    return static_cast<int>(msg.wParam);
}

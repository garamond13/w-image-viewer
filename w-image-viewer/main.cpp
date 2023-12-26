#include "pch.h"
#include "global.h"
#include "window.h"
#include "helpers.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    g_config.read();
    auto window{ std::make_unique<Window>(hInstance, nCmdShow) };

    // "direct" file open.
    if (lpCmdLine[0] != '\0') {
        [[maybe_unused]] int argc;
        auto argv{ CommandLineToArgvW(lpCmdLine, &argc) };
        window->renderer.user_interface.file_manager.file_open(argv[0]);
        wiv_assert(PostMessageW(g_hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
        LocalFree(argv);
    }

    // Message loop.
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

#include "pch.h"
#include "global.h"
#include "window.h"
#include "helpers.h"
#include "ensure.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    g_config.read();
    auto window = std::make_unique<Window>(hInstance, nCmdShow);

    // "direct" file open.
    if (lpCmdLine[0] != '\0') {
        int argc;
        auto argv = CommandLineToArgvW(lpCmdLine, &argc);
        window->renderer.ui.file_manager.file_open(argv[0]);
        ensure(PostMessageW(g_hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
        LocalFree(argv);
    }

    return window->message_loop();
}

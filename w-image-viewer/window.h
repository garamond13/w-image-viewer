#pragma once

#include "renderer.h"
#include "user_interface.h"
#include "file_manager.h"

// Window styles.
inline constexpr auto WIV_WINDOW_STYLE = WS_OVERLAPPEDWINDOW;
inline constexpr auto WIV_WINDOW_EX_STYLE = WS_EX_ACCEPTFILES;

// Window messages.
inline constexpr auto WIV_WM_OPEN_FILE = WM_USER + 0;
inline constexpr auto WIV_WM_RESET_RESOURCES = WM_USER + 1;

class Window
{
public:
	Window(HINSTANCE hinstance, int ncmdshow);
	int message_loop();
	Renderer renderer;
	bool is_minimized;
private:
	static LRESULT wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	void set_window_name() const;
	void reset_image_rotation() noexcept;
};

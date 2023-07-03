#pragma once

#include "renderer.h"
#include "user_interface.h"
#include "file_manager.h"

class Window
{
public:
	Window(HINSTANCE hinstance, int ncmdshow);
	Renderer renderer;
	HWND hwnd;
private:
	void set_window_name() const;
	void reset_image_rotation() noexcept;
	static LRESULT wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};

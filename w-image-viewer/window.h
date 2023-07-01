#pragma once

#include "config.h"
#include "renderer.h"
#include "user_interface.h"
#include "file_manager.h"

class Window
{
public:
	Window(Config* p_config, HINSTANCE hinstance, int ncmdshow);
	Renderer renderer;
	HWND hwnd;
private:
	void set_window_name();
	static LRESULT wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	Config* p_config;
};

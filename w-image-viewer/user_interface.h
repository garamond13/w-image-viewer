#pragma once

#include "pch.h"
#include "config.h"
#include "file_manager.h"

class User_interface
{
public:
	~User_interface();
	void create(Config* p_config, HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* device_context);
	void update();
	void draw() const;
	void input();
	void context_menu();
	void window_settings();
	void window_about();
	File_manager file_manager;
	bool is_fullscreen;
	bool is_dialog_file_open;
private:
	void dialog_file_open();
	void toggle_fullscreen();
	void auto_window_size() const;
	void dimm(bool condition = false) const;
	Config* p_config;
	HWND hwnd;
	bool is_window_settings_open;
	bool is_window_about_open;
};

#pragma once

#include "pch.h"
#include "config.h"
#include "file_manager.h"

class User_interface
{
public:
	~User_interface();
	void create(Config* p_config, HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* device_context, bool* should_update);
	void update();
	void draw() const;
	void auto_window_size() const;
	void reset_image_panzoom() noexcept;
	File_manager file_manager;
	bool is_fullscreen;
	bool is_dialog_file_open;
	std::pair<float, float> image_pan; //x,y
	float image_zoom;
	bool is_in_panzoom;

	//request to show image at the original width and height
	bool image_no_scale;

private:
	void input();
	void context_menu();
	void window_settings();
	void window_about();
	void dialog_file_open();
	void toggle_fullscreen();
	void dimm(bool condition = false) const;
	Config* p_config;
	HWND hwnd;
	bool is_window_settings_open;
	bool is_window_about_open;
	bool* p_renderer_should_update;
};

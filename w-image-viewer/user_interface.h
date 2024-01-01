#pragma once

#include "pch.h"
#include "file_manager.h"
#include "global.h"

enum WIV_OPEN_
{
	WIV_OPEN_IMAGE,
	WIV_OPEN_ICC
};

class User_interface
{
public:
	void create(ID3D11Device* device, ID3D11DeviceContext* device_context, bool* should_update);
	void update();
	void draw() const;
	void auto_window_size() const;
	void reset_image_panzoom() noexcept;
	~User_interface();
	File_manager file_manager;
	bool is_fullscreen;
	bool is_dialog_file_open;
	ImVec2 image_pan;
	bool is_panning;
	float image_zoom;
	bool is_zooming;
	float image_rotation;
	bool is_rotating;

	// Request to show image at the original width and height.
	bool image_no_scale;

private:
	void input();
	void overlay() const;
	void context_menu();
	void window_settings();
	void window_about();
	void dialog_file_open(WIV_OPEN_ file_type);
	void toggle_fullscreen();
	void dimm(bool condition = false) const;
	bool is_overlay_open = g_config.overlay_show.val;
	bool is_window_settings_open;
	bool is_window_about_open;
	bool* p_renderer_should_update;
};

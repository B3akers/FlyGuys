#pragma once
#include <Windows.h>
#include "imgui/imgui.h"
#include "vector.h"

namespace draw_manager
{
	extern ImDrawList* _drawList;

	void begin_rendering();
	void end_rendering();

	void add_text_on_screen(vector const& point, DWORD color, int font_size, const char* format, ...);
	void add_rect_on_screen(const vector& a, const vector& b, DWORD col, float rounding = 0.0f, int rounding_corners_flags = ~0, float thickness = 1.0f );
	void add_filled_rect_on_screen(const vector& a, const vector& b, DWORD col, float rounding = 0.0f, int rounding_corners_flags = ~0);
	vector calc_text_size(int font_size, const char* format, ...);
};
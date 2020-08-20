/* This file is part of FlyGuys by b3akers, licensed under the MIT license:
*
* MIT License
*
* Copyright (c) b3akers 2020
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "draw_manager.h"
#include "imgui/imgui_internal.h"
namespace draw_manager
{
	ImDrawList* _drawList;

	void begin_rendering()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::Begin("BackBuffer", reinterpret_cast<bool*>(true), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);

		ImGuiWindow* window = ImGui::GetCurrentWindow();

		_drawList = window->DrawList;
	}

	void end_rendering()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		window->DrawList->PushClipRectFullScreen();

		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}

	vector calc_text_size(int font_size, const char* format, ...)
	{
		static const auto MAX_BUFFER_SIZE = 1024;
		static char buffer[MAX_BUFFER_SIZE] = "";

		auto g = ImGui::GetCurrentContext();
		auto font = g->Font;
		int s = g->FontSize;

		g->FontSize = font_size;

		va_list va;
		va_start(va, format);
		vsnprintf_s(buffer, MAX_BUFFER_SIZE, format, va);
		va_end(va);

		auto ret = ImGui::CalcTextSize(buffer);

		g->FontSize = s;

		return { ret.x,ret.y };
	}

	void add_text_on_screen(vector const& point, DWORD color, int font_size, const char* format, ...)
	{
		va_list args_list;
		static char buffer[2048] = { 0 };

		va_start(args_list, format);
		vsnprintf(buffer, sizeof(buffer), format, args_list);
		va_end(args_list);

		auto g = ImGui::GetCurrentContext();
		auto font = g->Font;
		int s = g->FontSize;

		g->FontSize = font_size;
		_drawList->PushTextureID(font->ContainerAtlas->TexID);

		_drawList->AddText(font, font_size, { point.x, point.y }, color, buffer);
		_drawList->PopTextureID();
		g->FontSize = s;
	}

	void add_line_on_screen(vector const& start, vector const& end, DWORD color, float thickness)
	{
		_drawList->AddLine({ start.x, start.y }, { end.x, end.y }, color, thickness);
	}

	void add_rect_on_screen(const vector& a, const vector& b, DWORD col, float rounding, int rounding_corners_flags, float thickness)
	{
		_drawList->AddRect({ a.x,a.y }, { b.x,b.y }, col, rounding, rounding_corners_flags, thickness );
	}

	void add_filled_rect_on_screen(const vector& a, const vector& b, DWORD col, float rounding, int rounding_corners_flags)
	{
		_drawList->AddRectFilled({ a.x,a.y }, { b.x,b.y }, col, rounding, rounding_corners_flags);
	}
};
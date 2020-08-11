#pragma once
#include "imgui/imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
namespace ImGui
{
	bool Hotkey(const char* label, int* k, const ImVec2& size_arg = ImVec2(0, 0));
};


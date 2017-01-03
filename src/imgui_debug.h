
#pragma once

#include "linalg.h"

#define IMGUI_DEBUG(x) \
	ImGui::PushID(&x); \
	_ImGuiDebug(#x, x, &x); \
	ImGui::PopID()

void _ImGuiDebug(const char* label, int x, int* p);
void _ImGuiDebug(const char* label, float x, float* p);
void _ImGuiDebug(const char* label, const char* x, const char** p);

void _ImGuiDebug(const char* label, linalg::v2 x, linalg::v2* p);
void _ImGuiDebug(const char* label, linalg::v3 x, linalg::v3* p);
void _ImGuiDebug(const char* label, linalg::v4 x, linalg::v4* p);

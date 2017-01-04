
#pragma once

#include "linalg.h"

#define IMGUI_DEBUG(x) \
	ImGui::PushID(&x); \
	_ImGuiDebug(#x, x, &x); \
	ImGui::PopID()

void _ImGuiDebug(const char* label, int infer, int* p);
void _ImGuiDebug(const char* label, float infer, float* p);
void _ImGuiDebug(const char* label, const char* infer, const char** p);

void _ImGuiDebug(const char* label, linalg::v2i infer, linalg::v2i* p);
void _ImGuiDebug(const char* label, linalg::v2 infer, linalg::v2* p);
void _ImGuiDebug(const char* label, linalg::v3 infer, linalg::v3* p);
void _ImGuiDebug(const char* label, linalg::v4 infer, linalg::v4* p);

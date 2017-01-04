
#pragma once

#include "imgui_debug.h"
#include <imgui.h>
#include "linalg.h"

using namespace linalg;

void _ImGuiDebug(const char* label, int infer, int* p) { ImGui::DragInt(label, p); (void)infer; }
void _ImGuiDebug(const char* label, float infer, float* p) { ImGui::DragFloat(label, p); (void)infer; }
void _ImGuiDebug(const char* label, char* infer, char** p) { ImGui::InputText(label, *p, 64); (void)infer; }
void _ImGuiDebug(const char* label, const char* infer, const char** p) { ImGui::LabelText(label, *p); (void)infer; }

void _ImGuiDebug(const char* label, v2i infer, v2i* p) { ImGui::DragInt2(label, &p->x); (void)infer; }
void _ImGuiDebug(const char* label, v2 infer, v2* p) { ImGui::DragFloat2(label, &p->x); (void)infer; }
void _ImGuiDebug(const char* label, v3 infer, v3* p) { ImGui::DragFloat3(label, &p->x); (void)infer; }
void _ImGuiDebug(const char* label, v4 infer, v4* p) { ImGui::DragFloat4(label, &p->x); (void)infer; }

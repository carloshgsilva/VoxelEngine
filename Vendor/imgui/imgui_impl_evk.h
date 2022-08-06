#pragma once
#include "imgui.h"      // IMGUI_IMPL_API
#include <vulkan/vulkan.h>

IMGUI_IMPL_API bool     ImGui_ImplEvk_Init();
IMGUI_IMPL_API void     ImGui_ImplEvk_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplEvk_RenderDrawData(ImDrawData* draw_data);
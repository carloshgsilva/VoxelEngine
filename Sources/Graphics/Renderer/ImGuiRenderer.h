#pragma once

#include "Core/Engine.h"
#include "Graphics/Graphics.h"

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_evk.h"

#include "imgui/IconsFontAwesome5.h"

//TODO: Make it a Singleton (Module) as ImGui is already is singleton
class ImGuiRenderer {
	std::shared_ptr<struct InternalState> state;

public:

	ImGuiRenderer();
	~ImGuiRenderer();

	void SetStyle();

	const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	
	void OnGUI(std::function<void()> cb);
	void Draw();

	static const inline ImVec4 TextPrimary = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	static const inline ImVec4 TextSecondary = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
	static const inline ImVec4 Accent = ImVec4(0.0f, 0.6f, 0.0f, 1.0f);
	static const inline ImVec4 AccentDark = ImVec4(0.0f, 0.65f, 0.0f, 1.0f);
	static const inline ImVec4 PrimaryLight = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	static const inline ImVec4 PrimaryDark = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
	static const inline ImVec4 Border = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
};
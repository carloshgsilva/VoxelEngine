#include "EditorWindow.h"

#include "Editor/EditorLayer.h"

#include <imgui/imgui.h>
#include <glm/glm.hpp>

void EditorWindow::GUIInternal() {
	Focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	//Resize Event
	ImVec2 size = ImGui::GetWindowSize();
	if (size.x != _Width || size.y != _Height) {
		_Width = size.x;
		_Height = size.y;

		ViewportChangeEvent e;
		e.SizeX = _Width;
		e.SizeY = _Height;
		OnEvent(e);
	}

}

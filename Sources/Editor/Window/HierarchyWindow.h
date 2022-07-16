#pragma once

#include "EditorWindow.h"

#include "imgui/IconsFontAwesome5.h"

#include <entt/entt.hpp>

class HierarchyWindow : public EditorWindow {
	inline static constexpr const char* POPUP_CONTEXT = "HCPOPUP";

	int _CurrentIndex;

	//State used to Expand Select (Select using Shift)
	bool _ExpandSelectNextFrame{false};
	bool _ExpandSelectCurrentFrame{ false };
	bool _DeleteNextFrame{ false };
	int _SelectedIndexMin;
	int _SelectedIndexMax;

	void ContextMenu();
	void ImGui_EntityTree(entt::entity e);
public:

	virtual void OnEvent(Event& E) {}
	virtual void OnGUI();
	virtual void OnUpdate(float dt) {}

	virtual std::string GetName() { return ICON_FA_SITEMAP "  Hierarchy"; }
};

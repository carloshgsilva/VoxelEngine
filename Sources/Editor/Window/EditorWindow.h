#pragma once

#include "Event/Event.h"
#include "Graphics/Graphics.h"
#include <string>

class EditorLayer;

class EditorWindow {
	std::string _Name;
protected:
	int _Width = 0;
	int _Height = 0;
	bool Focused{ false };

public:
	EditorLayer* Editor;

	virtual ~EditorWindow() {}

	//On Event to handle inputs
	virtual void OnEvent(Event& E) {}

	//Used to make ImGui calls
	virtual void OnGUI() {}

	//Update per frame
	//TODO: Should be called only when focused?
	virtual void OnUpdate(float dt) {}

	//Should be called inside each EditorWindow::OnGUI() between ImGui::Begin() and ImGui::End();
	//This is used to correctly handle the window focus and viewport resize events
	void GUIInternal();

	virtual std::string GetName() { return "[UNNAMED_WINDOW]"; }
};

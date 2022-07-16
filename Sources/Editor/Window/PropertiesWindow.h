#pragma once

#include "EditorWindow.h"

class PropertiesWindow : public EditorWindow {
public:

	virtual void OnEvent(Event& E);
	virtual void OnGUI();
	virtual void OnUpdate(float dt);
	virtual std::string GetName();
};
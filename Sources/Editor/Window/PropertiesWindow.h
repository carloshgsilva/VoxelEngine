#pragma once

#include "EditorWindow.h"

class PropertiesWindow : public EditorWindow {
public:

	virtual void OnEvent(Event& E);
	virtual void OnGUI();
	virtual void OnUpdate(DeltaTime dt);
	virtual std::string GetName();
};
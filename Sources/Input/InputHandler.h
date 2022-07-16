#pragma once

#include "Event/Event.h"

class InputHandler {
	bool Keys[512];
	bool Buttons[64];

public:
	void OnEvent(Event& e) {
		if (e.Is<KeyEvent>()) {
			auto& E = e.As<KeyEvent>();
			Keys[(int)E.KeyCode] = E.Press;
		} 
		else if (e.Is<MouseButtonEvent>()) {
			auto& E = e.As<MouseButtonEvent>();
			Buttons[(int)E.MButton] = E.Press;
		}
	}

	bool IsKeyDown(Key InKey) {
		return Keys[(int)InKey];
	}
	bool IsButtonDown(Button InButton) {
		return Buttons[(int)InButton];
	}

	//TODO: OnKeyPress(key, std::function() _Cb) Events
};
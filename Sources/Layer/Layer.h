#pragma once

#include <string>

#include "Core/Core.h"
#include "Event/Event.h"

class Layer {

public:
	Layer(const std::string& name = "UnammedLayer") { _name = name; }
	virtual ~Layer() = default;

	virtual void OnAttach(){}
	virtual void OnDetach(){}
	virtual void OnUpdate(float dt){}
	virtual void OnEvent(Event& event){}

	const std::string& GetName() const { return _name; }

protected:
	std::string _name;
};
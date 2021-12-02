#pragma once

#include <string>

#include "Core/Core.h"
#include "Core/DeltaTime.h"
#include "Event/Event.h"

class Layer {

public:
	Layer(const std::string& name = "UnammedLayer") { _name = name; }
	virtual ~Layer() = default;

	virtual void OnAttach(){}
	virtual void OnDetach(){}
	virtual void OnUpdate(DeltaTime dt){}
	virtual void OnEvent(Event& event){}

	const std::string& GetName() const { return _name; }

protected:
	std::string _name;
};
#pragma once

#include "Event/Event.h"

#include <entt/entt.hpp>

class System {
public:
	//Only set after Constructor
	//to acces it use in the OnCreate or OnUpdate
	entt::registry* R;
	//Only set after Constructor
	//to acces it use in the OnCreate or OnUpdate
	class World* W;

	virtual void OnCreate() {}
	virtual void OnUpdate(float dt) {}
	virtual void OnEvent(Event& e) {}
	virtual void OnDestroy() {}
};

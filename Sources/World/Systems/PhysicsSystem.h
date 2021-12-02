#pragma once

#include "System.h"

#include <glm/vec3.hpp>

class PhysicsSystem : public System {
public:

	bool RayCast(glm::vec3 start, glm::vec3 dir, float& t, entt::entity& hitEntity);

	virtual void OnCreate() {}
	virtual void OnUpdate(DeltaTime dt) {}
	virtual void OnEvent(Event& e) {}
	virtual void OnDestroy() {}

};
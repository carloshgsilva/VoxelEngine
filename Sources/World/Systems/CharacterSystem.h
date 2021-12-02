#pragma once

#include "System.h"
#include <glm/glm.hpp>

class CharacterSystem : public System {

	void AttractEntity(entt::entity e, glm::vec3 target, float factor);

public:
	virtual void OnCreate() {}
	virtual void OnUpdate(DeltaTime dt);
	virtual void OnEvent(Event& e) {}
	virtual void OnDestroy() {}
};
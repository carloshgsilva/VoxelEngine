#pragma once

#include "System.h"

class IKSystem : public System {

	void OnChainUpdated(entt::registry& r, entt::entity e);

public:

	void Reset(entt::entity e);

	virtual void OnCreate();
	virtual void OnUpdate(float dt);
	virtual void OnEvent(Event& e) {}
	virtual void OnDestroy() {}
};
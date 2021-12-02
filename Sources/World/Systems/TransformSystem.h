#pragma once

#include "System.h"
#include "World/Components.h"

#include <entt/entt.hpp>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>


using entt::operator""_hs;
// Used to indicate that the Transform has changed
// and needs to have its matrix updated
// it means that the PreviosWorldMatrix and WorldMatrix are invalid
// Is automatically set when the Transform component is replaced
using Dirty = entt::tag<"Dirty"_hs>;

// Used to indicate that the Transform has changed
// you can use PreviosWorldMatrix and WorldMatrix safely
using Changed = entt::tag<"Changed"_hs>;

class TransformSystem : public System {

	void OnTransformCreated(entt::registry& r, entt::entity e);
	void OnTransformUpdated(entt::registry & r, entt::entity e);

public:

	virtual void OnCreate();

	virtual void OnUpdate(DeltaTime dt);

	//Returns the entity WorldMatrix
	//if the entity is Dirty recalculate it trough the hierarchy
	glm::mat4& GetWorldMatrix(entt::entity e);
	glm::vec3 GetWorldPosition(entt::entity e);
	void SetWorldPosition(entt::entity e, glm::vec3 pos);
	glm::quat GetWorldRotation(entt::entity e);
	void SetWorldRotation(entt::entity e, glm::quat rot);
	glm::vec3 WorldToLocal(glm::vec3 worldPoint, entt::entity space);
	glm::vec3 LocalToWorld(glm::vec3 localPoint, entt::entity space);

	void RealculateMatrix(entt::entity e, Transform& Trans, glm::mat4 ParentTransform = glm::mat4(1.0f));
};


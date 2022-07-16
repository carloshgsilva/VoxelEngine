#pragma once

#include "Event/Event.h"
#include "Graphics/Renderer/View.h"
#include "Input/InputHandler.h"

#include <glm/glm.hpp>

class EditorCamera {
public:
	float Fov{ 0.8f };
	float Near{ 0.1f };
	float Far{ 4096.0f };

	glm::vec3 Position;
	float Yaw, Pitch;
	glm::vec3 Velocity;

	glm::mat4 Matrix{1.0f};

	InputHandler _Input;
	bool _Moving;

	void OnEvent(Event& e);
	void Update(float dt);

	bool IsMoving() { return _Moving; }
	void SetMoving(bool value) { _Moving = value; }
	View GetView(float width, float height);

	glm::vec3 GetRight() { return glm::vec3(Matrix[0]); }
	glm::vec3 GetUp() { return glm::vec3(Matrix[1]); }
	glm::vec3 GetForward() { return glm::vec3(Matrix[2]); }
};
#pragma once

#include "Event/Event.h"
#include "Graphics/Renderer/View.h"
#include "Input/InputHandler.h"

#include <glm/glm.hpp>

class EditorCamera {
public:
	float fov = 0.8f;
	float near = 0.1f;
	float far = 4096.0f;

	float yaw = 0.0f;
	float pitch = 0.0f;

	glm::vec3 position = {};
	glm::vec3 velocity = {};
	glm::mat4 matrix = {};

	InputHandler input = {};
	bool moving = false;

	void OnEvent(Event& e);
	void Update(float dt);

	bool IsMoving() { return moving; }
	void SetMoving(bool value) { moving = value; }
	View GetView(uint32 width, uint32 height);

	glm::vec3 GetRight() { return glm::vec3(matrix[0]); }
	glm::vec3 GetUp() { return glm::vec3(matrix[1]); }
	glm::vec3 GetForward() { return glm::vec3(matrix[2]); }
};
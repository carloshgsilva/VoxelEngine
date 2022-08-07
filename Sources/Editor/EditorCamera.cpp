#include "EditorCamera.h"

#include "Core/Window.h"
#include "Core/Engine.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

inline constexpr float CAMERA_SPEED = 100.0f;
inline constexpr float CAMERA_MUL = 10.0f;
inline constexpr float CAMERA_RESISTANCE = 10.0f;

void EditorCamera::OnEvent(Event& e) {
	input.OnEvent(e);
	if (e.Is<MouseMoveEvent>()) {
		auto& E = e.As<MouseMoveEvent>();
		const float sensitivity = 0.001;
		if (input.IsButtonDown(Button::Right)) {
			pitch -= E.DeltaY * sensitivity;
			yaw -= E.DeltaX * sensitivity;
		}
	}
}

static double lastFullscreenTime = 0.0;
void EditorCamera::Update(float dt) {
	pitch = glm::clamp(pitch, -1.5f, 1.5f);

	glfwSetInputMode(Window::Get().GetNativeWindow(), GLFW_CURSOR, moving ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	
	//Only Move When Holding Right Mouse Button
	if (moving) {
		//Sprint
		float acc = CAMERA_SPEED * dt;
		if (input.IsKeyDown(Key::LeftShift)) {
			acc *= CAMERA_MUL;
		}

		//AWSD Movement
		if (input.IsKeyDown(Key::D)) { velocity += GetRight() * acc; }
		if (input.IsKeyDown(Key::A)) { velocity -= GetRight() * acc; }
		if (input.IsKeyDown(Key::E)) { velocity += GetUp() * acc; }
		if (input.IsKeyDown(Key::Q)) { velocity -= GetUp() * acc; }
		if (input.IsKeyDown(Key::S)) { velocity += GetForward() * acc; }
		if (input.IsKeyDown(Key::W)) { velocity -= GetForward() * acc; }
	}

	//Fullscreen Toggle
	if (input.IsKeyDown(Key::F) && ((Engine::GetTime() - lastFullscreenTime) > 1.0)) {
		lastFullscreenTime = Engine::GetTime();
		Window::Get().FullscreenToggle();
	}

	//Integrate Movement
	position += velocity * (float)dt;

	//Air Resistance
	velocity -= velocity * (float)dt * CAMERA_RESISTANCE;

	//Calculate Matrix
	matrix = glm::identity<glm::mat4>();
	matrix = glm::identity<glm::mat4>();
	matrix = glm::translate(matrix, position);
	matrix = glm::rotate(matrix, yaw, glm::vec3(0, 1, 0));
	matrix = glm::rotate(matrix, pitch, glm::vec3(1, 0, 0));
}

View EditorCamera::GetView(uint32 width, uint32 height) {
	View view = {};

	view.Position = position;
	view.CameraMatrix = matrix;
	view.ViewMatrix = glm::inverse(view.CameraMatrix);
	view.ProjectionMatrix = glm::perspective(fov, (float)width / (float)height, near, far);
	view.Width = width;
	view.Height = height;

	return view;
}

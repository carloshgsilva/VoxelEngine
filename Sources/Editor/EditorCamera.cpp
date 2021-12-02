#include "EditorCamera.h"

#include "Core/Window.h"
#include "Core/Engine.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

inline constexpr float CAMERA_SPEED = 100.0f;
inline constexpr float CAMERA_MUL = 10.0f;
inline constexpr float CAMERA_RESISTANCE = 10.0f;

void EditorCamera::OnEvent(Event& e) {
	_Input.OnEvent(e);
	if (e.Is<MouseMoveEvent>()) {
		auto& E = e.As<MouseMoveEvent>();
		const float sensitivity = 0.001;
		if (_Input.IsButtonDown(Button::Right)) {
			Pitch -= E.DeltaY * sensitivity;
			Yaw -= E.DeltaX * sensitivity;
		}
	}
}

static double lastFullscreenTime = 0.0;
void EditorCamera::Update(DeltaTime dt) {

	glfwSetInputMode(Window::Get().GetNativeWindow(), GLFW_CURSOR, _Moving ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

	//Only Move When Holding Right Mouse Button
	if (_Moving) {
		//Sprint
		float acc = CAMERA_SPEED * dt;
		if (_Input.IsKeyDown(Key::LeftShift)) {
			acc *= CAMERA_MUL;
		}

		//AWSD Movement
		if (_Input.IsKeyDown(Key::D)) { Velocity += GetRight() * acc; }
		if (_Input.IsKeyDown(Key::A)) { Velocity -= GetRight() * acc; }
		if (_Input.IsKeyDown(Key::E)) { Velocity += GetUp() * acc; }
		if (_Input.IsKeyDown(Key::Q)) { Velocity -= GetUp() * acc; }
		if (_Input.IsKeyDown(Key::S)) { Velocity += GetForward() * acc; }
		if (_Input.IsKeyDown(Key::W)) { Velocity -= GetForward() * acc; }
	}

	//Fullscreen Toggle
	if (_Input.IsKeyDown(Key::F) && ((Engine::GetTime() - lastFullscreenTime) > 1.0)) {
		lastFullscreenTime = Engine::GetTime();
		Window::Get().FullscreenToggle();
	}

	//Integrate Movement
	Position += Velocity * (float)dt;

	//Air Resistance
	Velocity -= Velocity * (float)dt * CAMERA_RESISTANCE;

	//Calculate Matrix
	Matrix = glm::identity<glm::mat4>();
	Matrix = glm::identity<glm::mat4>();
	Matrix = glm::translate(Matrix, Position);
	Matrix = glm::rotate(Matrix, Yaw, glm::vec3(0, 1, 0));
	Matrix = glm::rotate(Matrix, Pitch, glm::vec3(1, 0, 0));
}

View EditorCamera::GetView(float width, float height) {
	View view = {};

	view.Position = Position;
	view.CameraMatrix = Matrix;
	view.ViewMatrix = glm::inverse(view.CameraMatrix);
	view.ProjectionMatrix = glm::perspective(Fov, width / height, Near, Far);
	view.Width = width;
	view.Height = height;

	return view;
}

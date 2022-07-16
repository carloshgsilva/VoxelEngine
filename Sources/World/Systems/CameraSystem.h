#pragma once

#include "System.h"
#include "World/Components.h"

#include "Graphics/Renderer/View.h"
#include <entt/entt.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ECS {

	class CameraSystem : public System {

	public:

		static View CalculateView(Camera& Cam, Transform& Trans, float Width, float Height) {
			View view = {};

			view.Position = Trans.Position;
			view.CameraMatrix = Trans.WorldMatrix;
			view.ViewMatrix = glm::inverse(view.CameraMatrix);
			view.ProjectionMatrix = glm::perspective(Cam.Fov, Width / Height, Cam.Near, Cam.Far);
			view.Width = Width;
			view.Height = Height;

			return view;
		}
	};

}
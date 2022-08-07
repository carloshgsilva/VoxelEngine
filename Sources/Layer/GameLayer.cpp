#include "GameLayer.h"

#include "IO/Log.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Graphics/Pipelines/PresentPipeline.h"
#include "Graphics/Renderer/ImGuiRenderer.h"
#include "World/Systems/PhysicsSystem.h"
#include "Editor/Importer/VoxImporter.h"

GameLayer::GameLayer() : Layer("GameLayer") {
	_World = NewUnique<World>();

	//AssetImporter::Import("Mods/default/World.vox", "default");
	//AssetImporter::Import("Mods/default/World.vox", "default");
	AssetRefT<PrefabAsset> prefab = Assets::Load("default/island.pf");
	//AssetRefT<PrefabAsset> prefab = Assets::Load("default/FarmHouse.pf");
	//AssetRefT<PrefabAsset> prefab = Assets::Load("default/ModernHouse.pf");

	_World->SetLevel(prefab);

	_Camera = NewUnique<EditorCamera>();
	_Camera->position = { 26, 15, 25 };
	_Camera->yaw = 0.81f;
	_Camera->pitch = -0.43f;
	
	_WorldRenderer = NewUnique<WorldRenderer>();
	_WorldRenderer->RecreateFramebuffer(Window::Get().GetWidth(), Window::Get().GetHeight());
}

float camYVel = 0.0f;
void GameLayer::OnUpdate(float dt) {

	camYVel += 29.8f * dt;
	camYVel -= camYVel * 1.1f * dt;

	float t = 0.0f;
	entt::entity e;
	if (_World->Physics->RayCast(_Camera->position, glm::vec3(0, -1, 0), t, e) && t < 2.0f) {
		_Camera->position.y = _Camera->position.y + (1.5f - t)*dt*10.0f;
		camYVel = 0.0f;
		if (Input::IsKeyPressed(Key::Space)) {
			camYVel = -10.5f;
		}
	}

	_Camera->position.y -= camYVel * dt;

	if (Input::IsKeyPressed(Key::E)) {
		_Camera->position = { 26, 15, 25 };
	}

	_Camera->SetMoving(Input::IsButtonPressed(Button::Right));
	_Camera->Update(dt);
	_World->Update(dt);

	//TODO: 
	// Graphics::Frame([&]() {
	// 	View view = _Camera->GetView(_WorldRenderer->GetCurrentColorImage().getExtent().width, _WorldRenderer->GetCurrentColorImage().getExtent().height);
		
	// 	_WorldRenderer->DrawWorld(dt, view, *_World);

	// 	CmdPresent([&] {
	// 		PresentPipeline::Get().Use(_WorldRenderer->GetCurrentColorImage());
	// 	});
	// });

	
}

void GameLayer::OnEvent(Event& E) {
	_Camera->OnEvent(E);
	if (E.Is<ViewportChangeEvent>()) {
		auto& vpc = E.As<ViewportChangeEvent>();
		_WorldRenderer->RecreateFramebuffer(vpc.SizeX, vpc.SizeY);
	}
}

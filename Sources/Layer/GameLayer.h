#pragma once

#include "Layer.h"
#include "Core/Core.h"
#include "Graphics/Renderer/WorldRenderer.h"
#include "Editor/EditorCamera.h"

class GameLayer : public Layer{

	Unique<EditorCamera> _Camera;
	Unique<WorldRenderer> _WorldRenderer;
	Unique<World> _World;

public:
	GameLayer();
	virtual ~GameLayer() = default;

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnUpdate(DeltaTime dt);
	virtual void OnEvent(Event& event);
};
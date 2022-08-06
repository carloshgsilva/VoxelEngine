#pragma once

#include "EditorWindow.h"

#include "Graphics/Graphics.h"
#include "Graphics/Renderer/WorldRenderer.h"
#include "Editor/EditorCamera.h"
#include "Editor/EditorSelection.h"

#include "World/World.h"

#include <imgui/IconsFontAwesome5.h>

enum class ManipulateType {
	Translate,
	Rotate,
	Scale
};

// Shows a currently editing or playing scene
class ViewportWindow : public EditorWindow {
	ManipulateType _ManipulateType{ ManipulateType::Translate };
	Unique<EditorCamera> _Camera;
	Unique<WorldRenderer> _WorldRenderer;

	bool _IsDirty{ false };
	bool _playMode{ false };
	Unique<World> _World;
	Unique<World> _prefabWorld; //Stores the not changed world when in simulate mode
public:

	EditorSelection Selection;

	void DuplicateSelection();
	void DeleteSelection();

	Unique<WorldRenderer>& GetWorldRenderer() { return _WorldRenderer; }

	World* GetWorld() { return _World.get(); }
	void RenderWorld();

	void SetDirty() { _IsDirty = true; }
	bool IsDirty() { return _IsDirty; }

	ViewportWindow();
	virtual ~ViewportWindow() {}
	virtual void OnEvent(Event& E);
	virtual void OnGUI();
	virtual void OnUpdate(float dt);
	virtual std::string GetName();
};
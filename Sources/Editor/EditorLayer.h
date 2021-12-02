#pragma once

#include "Layer/Layer.h"

#include "World/World.h"
#include "Graphics/Renderer/ImGuiRenderer.h"

#include "Editor/EditorCommand.h"

#include "Editor/Window/EditorWindow.h"
#include "Editor/Window/AssetsWindow.h"
#include "Editor/Window/HierarchyWindow.h"
#include "Editor/Window/PropertiesWindow.h"
#include "Editor/Window/ViewportWindow.h"

#include <vector>
#include <entt/entt.hpp>



class EditorLayer : public Layer {
	ImGuiRenderer _imguiRenderer;

	EditorCommandStack _Commands;

	AssetsWindow* _Assets;
	HierarchyWindow* _Hierarchy;
	PropertiesWindow* _Properties;

public:

	std::vector<ViewportWindow*> Viewports;

	// Contains the current viewport
	// It's the windows that handle the current World
	// The last focused ViewportWindow
	//TODO: If is null, create a new empty prefab
	class ViewportWindow* Viewport;

	EditorLayer();
	~EditorLayer() {
		delete _Assets;
		delete _Hierarchy;
		delete _Properties;
		for (auto vp : Viewports) {
			delete vp;
		}
		Viewports.clear();
	}

	// Sets the Editor field in the Window
	template<typename T>
	T* RegisterWindow(T* window) { window->Editor = this; return window; }

	// Opens a new scene Viewport
	// if no parameters passed, create a empty scene
	void OpenViewport(AssetGUID asset = Asset::NullGUID);
	void CloseViewport(ViewportWindow* viewport);

	void OpenAsset(AssetGUID guid);

	void CmdGroup(std::function<void()> group) {
		int groupDepth = _Commands.GroupDepth+1;

		Cmd([&, groupDepth] {
			_Commands.GroupDepth++;
			if (!_Commands.IsLastCmd()) {
				while (_Commands.GroupDepth == groupDepth) { _Commands.Redo(); }
			}
		}, [&, groupDepth] {
			_Commands.GroupDepth--;
		});

		group();

		Cmd([&, groupDepth] {
			_Commands.GroupDepth--;
		}, [&, groupDepth] {
			_Commands.GroupDepth++;
			while (_Commands.GroupDepth == groupDepth) { _Commands.Undo(); }
		});
	}
	void Cmd(std::function<void()> Do, std::function<void()> Undo, std::string info = "") {
		EditorCommand cmd;
		cmd.Do = Do;
		cmd.Undo = Undo;
		cmd.Info = info;
		_Commands.RunCmd(cmd);
	}

	void OnGui();

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnUpdate(DeltaTime dt);
	virtual void OnEvent(Event& e);
};
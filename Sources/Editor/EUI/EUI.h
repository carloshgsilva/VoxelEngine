#pragma once

#include "Asset/Assets.h"

#include "Editor/EditorSelection.h"
#include "World/World.h"

#include <imgui/imgui.h>
#include <entt/entity/fwd.hpp>

namespace EUI {

	void AssetPreview();

	//Drag Drop
	static inline const char* Drag_Entity = "Entity";
	static inline const char* Drag_Asset_Vox = ".v";
	static inline const char* Drag_Asset_Pallete = ".p";
	static inline const char* Drag_Asset_Prefab = ".pf";
	static inline const char* Drag_Asset_Script = ".wren";

	bool DragEntitySource(EditorSelection& selection);
	EditorSelection* DragEntityTarget();

	void DragAssetSource(const std::string& assetPath);
	AssetGUID DragAssetTarget(const char* assetType);

	//Viewport
	static float VIEWPORT_BUTTON_SIZE = 18;
	static float VIEWPORT_BUTTON_PADDING = 4;
	void BeginOverlayGroup(int count);
	void EndOverlayGroup();
	bool WorldButton(const char* label, bool selected = false);

	//Gizmos
	void SetViewport(glm::mat4 view, glm::mat4 projection, ImVec2 min, ImVec2 max, ImDrawList* drawList);
	ImVec2 WorldToScreen(glm::vec3 pos);
	glm::vec3 ScreenToWorld(const ImVec2& pos);
	void DrawLine(const glm::vec3& start, const glm::vec3& end);
	void DrawVoxBounds(const glm::mat4& matrix, const glm::ivec3& size);
	void DrawSphere(const glm::vec3 pos, const float radius);

	//Entity Picker
	std::string EntityPickerSource(ImVec2 pos);
	void EntityPickerTarget(World* w, entt::entity e);
}
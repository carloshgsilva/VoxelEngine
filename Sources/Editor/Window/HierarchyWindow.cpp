#include "HierarchyWindow.h"

#include "Editor/EUI/EUI.h"
#include "Editor/EditorLayer.h"
#include "Editor/Window/ViewportWindow.h"
#include "World/Components.h"
#include "Graphics/Renderer/ImGuiRenderer.h"

#include <imgui/imgui.h>

void HierarchyWindow::ContextMenu() {
	auto* W = Editor->Viewport->GetWorld();
	auto& R = W->GetRegistry();

	if (ImGui::BeginMenu(ICON_FA_PLUS " Add")) {
		if (ImGui::MenuItem(ICON_FA_CUBE " Cube")) {
			auto e = W->Create("Cube");
			R.emplace<Transform>(e);
			R.emplace<VoxRenderer>(e);
			W->SetParent(e, Editor->Viewport->Selection.GetEntity());
			Editor->Viewport->Selection.Clear();
			Editor->Viewport->Selection.AddEntity(e);
		}
		if (ImGui::MenuItem(ICON_FA_LIGHTBULB " Light")) {
			auto e = W->Create("Light");
			R.emplace<Transform>(e);
			R.emplace<Light>(e);
			W->SetParent(e, Editor->Viewport->Selection.GetEntity());
			Editor->Viewport->Selection.Clear();
			Editor->Viewport->Selection.AddEntity(e);
		}
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem(ICON_FA_CLONE " Duplicate", "Crtl+D")) {
		Editor->Viewport->DuplicateSelection();
	}
	ImGui::MenuItem(ICON_FA_COPY " Copy", "Crtl+C");//TODO:
	ImGui::MenuItem(ICON_FA_PASTE " Paste", "Crtl+V");//TODO:
	if (ImGui::MenuItem(ICON_FA_TRASH " Delete", "delete")) {
		_DeleteNextFrame = true;
	}
}

void HierarchyWindow::ImGui_EntityTree(entt::entity e) {
	World* W = Editor->Viewport->GetWorld();
	entt::registry& R = W->GetRegistry();
	Hierarchy& h = R.get<Hierarchy>(e);
	
	bool hideChildren = false;
	bool isSelected = Editor->Viewport->Selection.IsSelected(e);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
	
	//Is Selected
	if (isSelected) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	//Make Root have no arrow and always open
	if (e == W->GetRoot()) {
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen;
	}

	//Prefabs is leaf
	if (R.has<Instance>(e) || h.Children.size() == 0) {
		flags |= ImGuiTreeNodeFlags_Leaf;
		//hideChildren = true;
	}
	

	bool open = false;
	if (R.has<Instance>(e) || h.Parent == entt::null) {
		open = ImGui::TreeNodeEx(fmt::format(ICON_FA_BOXES "  {}", R.get<Hierarchy>(e).Name).c_str(), flags);
	}
	else {
		open = ImGui::TreeNodeEx(fmt::format(ICON_FA_CUBE "  {}", R.get<Hierarchy>(e).Name).c_str(), flags);
	}
	
	//Entity Picker
	EUI::EntityPickerTarget(Editor->Viewport->GetWorld(), e);

	//Expand Selection
	if (_ExpandSelectCurrentFrame) {
		if (_CurrentIndex < _SelectedIndexMax && _CurrentIndex > _SelectedIndexMin) {
			Editor->Viewport->Selection.AddEntity(e);
		}
	}
	else {
		if (isSelected) {
			_SelectedIndexMin = std::min(_SelectedIndexMin, _CurrentIndex);
			_SelectedIndexMax = std::max(_SelectedIndexMax, _CurrentIndex);
		}
	}
	
	//DragAndDrop entity Source
	if (EUI::DragEntitySource(Editor->Viewport->Selection) && !isSelected) {
		//Select Only the current entity if dragged and not selected
		Editor->Viewport->Selection.Clear();
		Editor->Viewport->Selection.AddEntity(e);
		
	}
	//DragAndDrop entity Target
	EditorSelection* draggedEntities = EUI::DragEntityTarget();
	if (draggedEntities) {
		for (auto draggedEntity : draggedEntities->GetEntities()) {
			if (!W->IsSuperParent(e, draggedEntity)) {
				W->SetParent(draggedEntity, e);
			}
		}
	}
	
	//Selection Behaviour
	if (ImGui::IsItemHovered() && (ImGui::IsItemClicked(ImGuiMouseButton_Left)|| ImGui::IsItemClicked(ImGuiMouseButton_Right)) && !ImGui::IsItemToggledOpen()) {
		if (ImGui::GetIO().KeyCtrl) {
			Editor->Viewport->Selection.ToggleEntity(e);
		}
		else if (ImGui::GetIO().KeyShift) {
			_ExpandSelectNextFrame = true;
			_SelectedIndexMin = std::min(_SelectedIndexMin, _CurrentIndex);
			_SelectedIndexMax = std::max(_SelectedIndexMax, _CurrentIndex);
			Editor->Viewport->Selection.AddEntity(e);
		}
		else {
			Editor->Viewport->Selection.Clear();
			Editor->Viewport->Selection.AddEntity(e);
		}
	}

	//Context Menu
	ImGui::PushID(R.get<Hierarchy>(e).Name.c_str());
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		ImGui::OpenPopup(POPUP_CONTEXT);
	}
	if (ImGui::BeginPopup(POPUP_CONTEXT)) {
		ContextMenu();
		ImGui::EndPopup();
	}
	ImGui::PopID();

	//DragAndDrop PrefabAsset Target
	AssetGUID prefabToSpawn = EUI::DragAssetTarget(EUI::Drag_Asset_Prefab);
	if (prefabToSpawn != Asset::NullGUID) {
		AssetRefT<PrefabAsset> prefab = Assets::Load(prefabToSpawn);
		if (prefab.IsValid()) {
			prefab->Spawn(W, e);
		}
		else {
			//TODO: Show Notif or Log
		}
	}
	

	_CurrentIndex++;

	if (open) {
		if (!hideChildren) {
			W->ForEachChild(e, [&](entt::entity c) {
				ImGui_EntityTree(c);
			});
		}

		ImGui::TreePop();
	}
}

void HierarchyWindow::OnGUI() {

	_CurrentIndex = 1;

	//Will Expand in this frame min and max are set
	if (_ExpandSelectCurrentFrame) {
		_ExpandSelectCurrentFrame = false;
	}

	if (_ExpandSelectNextFrame) {
		_ExpandSelectCurrentFrame = true;
		_ExpandSelectNextFrame = false;
	}
	//If will not expand in next frame reset min and max
	else {
		_SelectedIndexMin = 9999999;
		_SelectedIndexMax = 0;
	}

	if (_DeleteNextFrame) {
		_DeleteNextFrame = false;
		Editor->Viewport->DeleteSelection();
	}

	ImGui::Begin(GetName().c_str());
	GUIInternal();

	World* W = Editor->Viewport->GetWorld();
	
	ImGui::PushStyleColor(ImGuiCol_Header, ImGuiRenderer::Accent);

	if (W) {
		//ImGui_EntityTree(W->GetRoot());
		
		entt::registry& R = W->GetRegistry();
		R.each([&](const entt::entity e) {

			if (R.get<Hierarchy>(e).Parent == entt::null) {
				ImGui_EntityTree(e);
			}
		});
		
	} else {
		ImGui::Text("No World Open!");
	}

	ImGui::PopStyleColor();

	ImGui::End();
}

#include "PropertiesWindow.h"


#include "Editor/EditorLayer.h"
#include "Editor/Window/ViewportWindow.h"
#include "Mod/ModLoader.h"

#include "World/Components.h"
#include "Graphics/Renderer/ImGuiRenderer.h"
#include "Editor/EUI/EUI.h"

#include <entt/entt.hpp>
#include <imgui/imgui_internal.h>
#include <imgui/IconsFontAwesome5.h>
#include <imgui/imgui.h>


#define DO_UNDO(type, comp, member) \
	Editor->Cmd( \
		[=, &R, newValue = c.member](){ \
			comp& c = R.get<comp>(e); \
			c.member = newValue; \
		}, \
			[=, &R, oldValue = GUI::Last##type](){ \
			comp& c = R.get<comp>(e); \
			c.member = oldValue; \
		}, \
		"Set " #comp " " #member \
	); \

#define DO_UNDO_ADD_COMPONENT(componentName) \
if (!R.has<componentName>(e) && ImGui::MenuItem(#componentName)) { \
	Editor->Cmd( \
		[=, &R]() { \
		R.emplace<componentName>(e); \
	}, \
		[=, &R]() { \
		R.remove<componentName>(e); \
	}, "Add " #componentName " Component"); \
} \

#define DO_UNDO_REMOVE_COMPONENT(componentName) \
if (GUI::RemoveComponent) { \
	Editor->Cmd( \
		[=, &R]() { \
			Log::info("Remove Component " #componentName " do!");\
		R.remove<componentName>(e); \
	}, \
		[=, &R, oldValue = R.get<componentName>(e)](){ \
			Log::info("Remove Component " #componentName " undo!");\
		R.emplace<componentName>(e, oldValue); \
	}, \
		"Remove " #componentName " Component" \
	); \
} \

namespace GUI {
	//Tells if some of the component has changed
	//Useful if you want to notify per frame changing
	static bool ComponentChanged = false;
	static bool RemoveComponent = false;

	bool BeginComponent(const char* title) {
		ComponentChanged = false;
		RemoveComponent = false;

		//Header
		bool open = false;
		ImGui::GetCurrentWindow()->WorkRect.Max.x -= 32.0f;
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImGuiRenderer::Accent);
			open = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::PopStyleColor();
		}
		ImGui::GetCurrentWindow()->WorkRect.Max.x += 32.0f;

		//Close Button
		{
			ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0,0,0,0));
			ImVec2 oldPos = ImGui::GetCursorScreenPos();
			ImVec2 thePos{ 
				ImGui::GetItemRectMax().x + 4.0f,
				ImGui::GetItemRectMin().y,
			};
			ImGui::SetCursorScreenPos(thePos);

			ImGui::PushID(title);
			if (ImGui::Button("X")) {
				RemoveComponent = true;
				Log::info("Remove Component Click!");
			}
			ImGui::PopID();
			ImGui::SetCursorScreenPos(oldPos);
			ImGui::PopStyleColor();
		}

		//if (open) {
			auto frameHeight = ImGui::GetFrameHeight();
			auto newPos = ImGui::GetCursorScreenPos();
			newPos.x += frameHeight;
			ImGui::SetCursorScreenPos(newPos);

			ImGui::BeginGroup();
		//}

		return open;
	}
	void EndComponent() {
		ImGui::EndGroup();

		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(ImGui::GetItemRectMin().x - 8.0f, ImGui::GetItemRectMin().y),
			ImVec2(ImGui::GetItemRectMin().x - 8.0f , ImGui::GetItemRectMax().y),
			ImColor(ImGuiRenderer::Accent)
		);
	}
	
	void PropertyLabel(const char* label) {
		float width = 70.0f;//ImGui::GetContentRegionAvail().x / 3;

		ImVec2 min = ImGui::GetCursorScreenPos();
		//min.x += width/2.0f - ImGui::CalcTextSize(label).x / 2.0f;
		ImVec2 max = min;
		max.x += width;
		max.y += ImGui::GetFontSize();

		const char* labelEnd = label + strlen(label);
		ImVec2 labelSize = ImGui::CalcTextSize(label, labelEnd);

		//ImGui::AlignTextToFramePadding();
		ImGui::RenderTextEllipsis(ImGui::GetWindowDrawList(), min, max, max.x, max.x, label, labelEnd, &labelSize);
		ImGui::Dummy(ImVec2(width, ImGui::GetFontSize()));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	}

	//Here begins the properties editor, it returns true if it was edited to add
	//some sort of undo/redo
	// use Last**** to access the value before edit

	static std::string LastNameString = std::string(256, '\0');
	int _NamePropCallback(ImGuiInputTextCallbackData* data) {
		constexpr int MAX_NAME_SIZE = 32;
		std::string* str = (std::string*)data->UserData;
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
			if (data->BufTextLen <= MAX_NAME_SIZE) {
				str->resize(data->BufTextLen);
				data->Buf = str->data();
			}
		}
		else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) {
			if (data->BufTextLen > MAX_NAME_SIZE) {
				*(data->Buf + MAX_NAME_SIZE) = '\0';
				data->BufTextLen = MAX_NAME_SIZE;
				data->BufDirty = true;
			}
		}

		return 0;
	}
	bool NameProperty(std::string& v) {
		if (ImGui::InputText("##NAME", v.data(), v.size() + 1, ImGuiInputTextFlags_CallbackResize| ImGuiInputTextFlags_CallbackEdit, &_NamePropCallback, &v)) {
			
		}

		if (ImGui::IsItemActivated()) { LastNameString = v; }
		if (ImGui::IsItemActive()) { ComponentChanged = true; }

		return ImGui::IsItemDeactivatedAfterEdit();
	}

	static int LastInt = 0;
	bool Property(const char* title, int& v) {
		PropertyLabel(title);
		ImGui::PushID(title);
		ImGui::InputInt("##EMPTY", (int*)&v);
		ImGui::PopID();

		if (ImGui::IsItemActivated()) { LastInt = v; }
		if (ImGui::IsItemActive()) { ComponentChanged = true; }

		return ImGui::IsItemDeactivatedAfterEdit();
	}

	static int LastFloat = 0;
	bool Property(const char* title, float& v) {
		PropertyLabel(title);
		ImGui::PushID(title);
		ImGui::DragFloat("##EMPTY", (float*)&v, 0.1f);
		ImGui::PopID();

		if (ImGui::IsItemActivated()) { LastFloat = v; }
		if (ImGui::IsItemActive()) { ComponentChanged = true; }

		return ImGui::IsItemDeactivatedAfterEdit();
	}

	static std::string LastEntitySlot;
	bool Property(const char* title, EntitySlot& v) {
		PropertyLabel(title);
		ImGui::PushID(title);
		
		const float drag_width = 16.0f;
		auto& style = ImGui::GetStyle();
		
		ImVec2 start = ImGui::GetCursorScreenPos();
		float width = ImGui::CalcItemWidth();
		float height = ImGui::GetFrameHeight();
		ImVec2 end = ImVec2(start.x+width, start.y+height);
		ImRect bb = ImRect(start, end);

		ImGui::ItemSize(bb, style.FramePadding.y);
		ImGui::ItemAdd(bb, ImGui::GetID(title));

		ImGui::RenderFrame(start, end, ImGui::GetColorU32(ImGui::IsItemHovered() ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
		ImGui::RenderText(ImVec2(bb.Min.x + style.ItemInnerSpacing.x + height, bb.Min.y + style.FramePadding.y), v.GetPath().c_str());

		ImVec2 dot_center = ImVec2(bb.Min.x + style.ItemInnerSpacing.x + drag_width * 0.5f, bb.Min.y + height * 0.5f);
		ImGui::GetWindowDrawList()->AddCircle(dot_center, 6, ImGui::GetColorU32(ImGuiCol_Text), 16, 1.5f);
		ImGui::GetWindowDrawList()->AddCircleFilled(dot_center, 2, ImGui::GetColorU32(ImGuiCol_Text), 8);
		
		auto pick = EUI::EntityPickerSource(dot_center);
		if (!pick.empty()) {
			v.SetPath(pick);
		}

		ImGui::PopID();

		EditorSelection* drop = EUI::DragEntityTarget();
		if (drop) {
			v.SetPath(drop->GetWorld()->GetEntityPath(drop->GetEntity()));
		}

		if (ImGui::IsItemActivated()) { LastEntitySlot = v.GetPath(); }
		if (ImGui::IsItemActive()) { ComponentChanged = true; }

		return ImGui::IsItemDeactivatedAfterEdit();
	}


	static glm::vec3 LastVec3=glm::vec3(0.0f);
	bool Property(const char* title, glm::vec3& vec) {
		PropertyLabel(title);
		ImGui::PushID(title);
		ImGui::DragFloat3("##EMPTY", (float*)&vec, 0.1f, -FLT_MAX, FLT_MAX, "%.2f");
		ImGui::PopID();

		if (ImGui::IsItemActivated()) { LastVec3 = vec; }
		if (ImGui::IsItemActive()) { ComponentChanged = true; }

		return ImGui::IsItemDeactivatedAfterEdit();
	}

	static glm::vec3 LastLightColor = glm::vec3();
	bool PropertyColor(const char* title, glm::vec3& v) {
		PropertyLabel(title);
		ImGui::PushID(title);
		ImGui::ColorEdit3("##EMPTY", (float*)&v);
		ImGui::PopID();

		if (ImGui::IsItemActivated()) { LastLightColor = v; }
		if (ImGui::IsItemActive()) { ComponentChanged = true; }

		return ImGui::IsItemDeactivatedAfterEdit();
	}
	
	static Light::Type LastLightType = Light::Type::Point;
	bool Property(const char* title, Light::Type& v) {
		PropertyLabel(title);

		constexpr int LightTypesCount = 4;
		constexpr const char* LightTypesAliases[LightTypesCount] = {
			"Point",
			"Spot",
			"Directional",
			"Ambient",
		};

		bool changed = false;
		if (ImGui::BeginCombo("##EMPTY", LightTypesAliases[(int)v])) {
			for (int i = 0; i < LightTypesCount; i++) {
				const char* lightAlias = LightTypesAliases[i];
				if (ImGui::Selectable(lightAlias)) {
					v = (Light::Type)i;
					changed = true;
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::IsItemActivated()) { LastLightType = v; }
		if (ImGui::IsItemActive()) { ComponentChanged = true; }

		return changed;
	}

	// Assets

	void AssetSelector(const char* title, AssetGUID guid) {
		ImGui::BeginGroup();
		{
			ImGui::Dummy(ImVec2(0, 24 - ImGui::GetFontSize() * 0.5f));
			PropertyLabel(title);
		}
		ImGui::EndGroup();

		ImGui::SameLine();
		
		ImGui::BeginGroup();
		{
			EUI::AssetPreview();
			ImGui::SameLine();
			ImGui::BeginGroup();
			{
				ImGui::Dummy(ImVec2(0, 8));
				const std::string& path = ModLoader::Get().GetAssetPath(guid);
				ImGui::Text(path.c_str() + path.find_last_of('/') + 1);
				ImGui::Text(path.c_str());
			}
			ImGui::EndGroup();
		}
		ImGui::EndGroup();
	}

	static AssetSlot<VoxAsset> LastVoxAsset;
	bool Property(const char* title, AssetSlot<VoxAsset>& asset) {
		AssetSelector(title, asset.GetGUID());

		AssetGUID newAsset = EUI::DragAssetTarget(EUI::Drag_Asset_Vox);
		if (newAsset) {
			LastVoxAsset = asset;
			asset.SetGUID(newAsset);

			ComponentChanged = true;
			return true;
		}

		return false;
	}

	static AssetSlot<PalleteAsset> LastPalleteAsset;
	bool Property(const char* title, AssetSlot<PalleteAsset>& asset) {
		AssetSelector(title, asset.GetGUID());

		AssetGUID newAsset = EUI::DragAssetTarget(EUI::Drag_Asset_Pallete);
		if (newAsset) {
			LastPalleteAsset = asset;
			asset.SetGUID(newAsset);

			ComponentChanged = true;
			return true;
		}

		return false;
	}
	
	static AssetSlot<ScriptAsset> LastScriptAsset;
	bool Property(const char* title, AssetSlot<ScriptAsset>& asset) {
		AssetSelector(title, asset.GetGUID());

		AssetGUID newAsset = EUI::DragAssetTarget(EUI::Drag_Asset_Script);
		if (newAsset) {
			LastScriptAsset = asset;
			asset.SetGUID(newAsset);

			ComponentChanged = true;
			return true;
		}

		return false;
	}

}
void PropertiesWindow::OnEvent(Event& E) {
	if (E.Is<FocusChangeEvent>()) {
		if (E.As<FocusChangeEvent>().Focused) {
			//Editor->SelectedEntity = entt::null;
		}
	}
}

void PropertiesWindow::OnGUI() {

	ImGui::Begin(GetName().c_str());
	GUIInternal();

	World* W = Editor->Viewport->GetWorld();
	entt::registry& R = W->GetRegistry();


	if (Editor->Viewport->Selection.HasSelection()){
		entt::entity e = Editor->Viewport->Selection.GetEntity();

		if (!R.valid(e)) {
			Editor->Viewport->Selection.RemoveEntity(Editor->Viewport->Selection.GetEntity());
			return;
		}

		if (R.has<Instance>(e)) {
			ImGui::Text(ICON_FA_BOXES);
		}
		else {
			ImGui::Text(ICON_FA_CUBES);
		}
		ImGui::SameLine();

		GUI::NameProperty(R.get<Hierarchy>(e).Name);

		if (R.has<Instance>(e)) {
			const std::string& assetPath = ModLoader::Get().GetAssetPath(R.get<Instance>(e).Prefab.GetGUID());
			ImGui::Text(fmt::format("Prefab: {}", assetPath).c_str());
			if (ImGui::Button(ICON_FA_PEN " Edit Prefab", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
				Editor->OpenViewport(R.get<Instance>(e).Prefab.GetGUID());
				goto finish;
			}
			if (!R.get<Instance>(e).Prefab.IsValid()) {
				R.remove<Instance>(e);
			}
		}
		
		if (R.has<Transform>(e)) {
			if (GUI::BeginComponent("Transform")) {
				Transform& c = R.get<Transform>(e);
				DO_UNDO_REMOVE_COMPONENT(Transform);

				if (GUI::Property("Position", c.Position)) { DO_UNDO(Vec3, Transform, Position); }
				if (GUI::Property("Rotation", c.Rotation)) { DO_UNDO(Vec3, Transform, Rotation); }
				if (GUI::Property("Scale", c.Scale)) { DO_UNDO(Vec3, Transform, Scale); }

				//Used to dirty transform
				if (GUI::ComponentChanged) {
					R.replace<Transform>(e, R.get<Transform>(e));
				}
			}
			GUI::EndComponent();
		}
		if (R.has<VoxRenderer>(e)) {
			if (GUI::BeginComponent("VoxRenderer")) {
				VoxRenderer& c = R.get<VoxRenderer>(e);
				DO_UNDO_REMOVE_COMPONENT(VoxRenderer);

				if (GUI::Property("Vox", c.Vox)) { DO_UNDO(VoxAsset, VoxRenderer, Vox); }
				if (GUI::Property("Pallete", c.Pallete)) { DO_UNDO(VoxAsset, VoxRenderer, Vox); }
				if (GUI::Property("Pivot", c.Pivot)) { DO_UNDO(Vec3, VoxRenderer, Pivot); }

				if (GUI::ComponentChanged) {
					c.VoxSlot = -1;
				}
			}
			GUI::EndComponent();
		}
		if (R.has<Light>(e)) {
			if (GUI::BeginComponent("Light")) {
				Light& c = R.get<Light>(e);
				DO_UNDO_REMOVE_COMPONENT(Light);

				if (GUI::Property("Type", c.LightType)) { DO_UNDO(LightType, Light, LightType); }
				if (GUI::PropertyColor("Color", c.Color)) { DO_UNDO(LightColor, Light, Color); }
				if (GUI::Property("Intensity", c.Intensity)) { DO_UNDO(Float, Light, Intensity); }
				if (GUI::Property("Attenuation", c.Attenuation)) { DO_UNDO(Float, Light, Attenuation); }
				if (GUI::Property("Range", c.Range)) { DO_UNDO(Float, Light, Range); }
				
				if (c.LightType == Light::Type::Spot) {
					if (GUI::Property("Angle", c.Angle)) { DO_UNDO(Float, Light, Angle); }
					if (GUI::Property("AngleAttenuation", c.AngleAttenuation)) { DO_UNDO(Float, Light, AngleAttenuation); }
				}
			}
			GUI::EndComponent();
		}
		if (R.has<Script>(e)) {
			if (GUI::BeginComponent("Script")) {
				Script& c = R.get<Script>(e);
				DO_UNDO_REMOVE_COMPONENT(Script);

				if (GUI::Property("Asset", c.Asset)) { DO_UNDO(ScriptAsset, Script, Asset); }
				if (GUI::ComponentChanged) {
					R.replace<Script>(e, R.get<Script>(e));
				}

			}
			GUI::EndComponent();
		}
		if (R.has<Character>(e)) {
			if (GUI::BeginComponent("Character")) {
				Character& c = R.get<Character>(e);
				DO_UNDO_REMOVE_COMPONENT(Character);

				//if (GUI::Property("Asset", c.Asset)) { DO_UNDO(ScriptAsset, Script, Asset); }
				//if (GUI::ComponentChanged) {
				//	R.replace<Script>(e, R.get<Script>(e));
				//}

			}
			GUI::EndComponent();
		}
		if (R.has<IKChain>(e)) {
			if (GUI::BeginComponent("IKChain")) {
				IKChain& c = R.get<IKChain>(e);
				DO_UNDO_REMOVE_COMPONENT(IKChain);
				if (GUI::Property("Target", c.Target)) { DO_UNDO(EntitySlot, IKChain, Target); }
				if (GUI::Property("Pole", c.Pole)) { DO_UNDO(EntitySlot, IKChain, Pole); }
				if (GUI::Property("Depth", c.Depth)) { DO_UNDO(Int, IKChain, Depth); }

				if (GUI::ComponentChanged) {
					R.replace<IKChain>(e, c);
				}
			}
			GUI::EndComponent();
		}

		ImGui::Spacing();

		//Add Component
		{
			if (ImGui::Button(ICON_FA_PLUS " Add Component", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
				ImGui::OpenPopup("Add Component");
			}

			if (ImGui::BeginPopup("Add Component")) {

				DO_UNDO_ADD_COMPONENT(Transform);
				DO_UNDO_ADD_COMPONENT(Light);
				DO_UNDO_ADD_COMPONENT(VoxRenderer);
				DO_UNDO_ADD_COMPONENT(Script);
				DO_UNDO_ADD_COMPONENT(Character);
				DO_UNDO_ADD_COMPONENT(IKChain);

				ImGui::MenuItem("Camera");
				ImGui::EndPopup();
			}
		}

	}

	finish:
	ImGui::End();
}

void PropertiesWindow::OnUpdate(float dt) {
}

std::string PropertiesWindow::GetName() {
	return ICON_FA_INFO " Properties";
}

#include "EUI.h"

#include "World/World.h"
#include "World/Components.h"
#include "Graphics/Renderer/ImGuiRenderer.h"

#include <entt/entt.hpp>
#include <imgui/imgui_internal.h>


void EUI::AssetPreview() {
	const float WIDTH = 60.0f;
	ImGui::Dummy(ImVec2(WIDTH, WIDTH));
	ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(255, 255, 255, 50), 4.0f);
}


bool EUI::DragEntitySource(EditorSelection& selection) {
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		void* data = &selection;
		ImGui::SetDragDropPayload(Drag_Entity, &data, sizeof(EditorSelection*), ImGuiCond_Always);
		for (auto e : selection.GetEntities()) {
			ImGui::Text(selection.GetWorld()->GetRegistry().get<Hierarchy>(e).Name.c_str());
		}
		ImGui::EndDragDropSource();
		return true;
	}
	return false;
}
EditorSelection* EUI::DragEntityTarget() {
	if (!ImGui::GetDragDropPayload() || !ImGui::GetDragDropPayload()->IsDataType(Drag_Entity))
		return nullptr;

	ImVec2 min = ImGui::GetItemRectMin();
	min.x -= 4.0;
	min.y -= 4.0;
	ImVec2 max = ImGui::GetItemRectMax();
	max.x += 4.0;
	max.y += 4.0;

	if (ImGui::BeginDragDropTarget()) {
		if (ImGui::AcceptDragDropPayload(Drag_Entity)) {
			EditorSelection* selection = *(EditorSelection**)ImGui::GetDragDropPayload()->Data;
			return selection;
		}
		ImGui::EndDragDropTarget();
	}
	/*
	* For now the hint highlight is just disabled but maybe we will need it when doing EntityRef
	else {
		if (ImGui::GetDragDropPayload() && ImGui::GetDragDropPayload()->IsDataType(Drag_Entity)) {
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(0, 255, 0, 80), 4.0f);
		}
	}
	*/

	return nullptr;
}

void EUI::DragAssetSource(const std::string& assetPath) {

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		auto dotIndex = assetPath.find_last_of('.');
		ImGui::SetDragDropPayload(assetPath.c_str() + dotIndex, assetPath.c_str(), assetPath.size()+1, ImGuiCond_Always);
		ImGui::TextUnformatted(assetPath.c_str());
		ImGui::EndDragDropSource();
	}
}
AssetGUID EUI::DragAssetTarget(const char* assetType) {
	if (!ImGui::GetDragDropPayload() || !ImGui::GetDragDropPayload()->IsDataType(assetType))
		return Asset::NullGUID;

	ImVec2 min = ImGui::GetItemRectMin();
	min.x -= 4.0;
	min.y -= 4.0;
	ImVec2 max = ImGui::GetItemRectMax();
	max.x += 4.0;
	max.y += 4.0;

	if (ImGui::BeginDragDropTarget()) {
		if (ImGui::AcceptDragDropPayload(assetType)) {
			return Assets::Hash((const char*)ImGui::GetDragDropPayload()->Data);
		}
		ImGui::EndDragDropTarget();
	}
	else {
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(0, 255, 0, 80), 4.0f);
	}

	return Asset::NullGUID;
}

void EUI::BeginOverlayGroup(int count) {
	ImVec2 min = ImGui::GetCursorScreenPos();
	ImVec2 max = min;
	max.x += count * (VIEWPORT_BUTTON_SIZE)+(count*2)* VIEWPORT_BUTTON_PADDING;
	max.y += VIEWPORT_BUTTON_SIZE + VIEWPORT_BUTTON_PADDING * 2.0f;

	ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(60, 60, 60, 220), VIEWPORT_BUTTON_SIZE / 2 + VIEWPORT_BUTTON_PADDING);

	min.x += VIEWPORT_BUTTON_PADDING;
	min.y += VIEWPORT_BUTTON_PADDING;
	ImGui::SetCursorScreenPos(min);
}
void EUI::EndOverlayGroup() {
	ImVec2 c = ImGui::GetCursorScreenPos();
	c.y -= 4.0f;
	ImGui::SetCursorScreenPos(c);
}
bool EUI::WorldButton(const char* label, bool selected) {
	bool clicked = false;
	ImVec2 intialPos = ImGui::GetCursorScreenPos();

	if (ImGui::InvisibleButton(label, ImVec2(VIEWPORT_BUTTON_SIZE, VIEWPORT_BUTTON_SIZE))) {
		clicked = true;
	}

	bool hovered = ImGui::IsItemHovered();
	bool held = ImGui::IsMouseDown(ImGuiMouseButton_Left);
	ImU32 color = selected ? ImGui::ColorConvertFloat4ToU32(ImGuiRenderer::Accent) : hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(200, 200, 200, 255);
	
	ImVec2 pos = intialPos;
	ImVec2 offset = ImGui::CalcTextSize(label);
	pos.x += (VIEWPORT_BUTTON_SIZE - offset.x)*0.5f;
	pos.y += (VIEWPORT_BUTTON_SIZE - offset.y)*0.5f;

	ImGui::SetCursorScreenPos(pos);

	ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(color), label);
	
	pos = intialPos;
	pos.x += VIEWPORT_BUTTON_SIZE + VIEWPORT_BUTTON_PADDING*2.0f;
	ImGui::SetCursorScreenPos(pos);

	return clicked;
}

glm::mat4 VP_ViewProjection{1.0f};
ImVec2 VP_Min;
ImVec2 VP_Size;
ImDrawList* VP_DrawList;
void EUI::SetViewport(glm::mat4 view, glm::mat4 projection, ImVec2 min, ImVec2 size, ImDrawList* drawList) {
	VP_ViewProjection = projection * view;
	VP_Min = min;
	VP_Size = size;
	VP_DrawList = drawList;
}
ImVec2 EUI::WorldToScreen(glm::vec3 pos) {
	glm::vec4 p = VP_ViewProjection * glm::vec4(pos, 1.0f);
	if (p.w <= 0.0f) {
		p.w = 0.000001f;
		//return ImVec2(0.0f, 0.0f);
	} 
	p.x /= p.w;
	p.y /= p.w;
	return ImVec2((p.x * 0.5f + 0.5f) * VP_Size.x + VP_Min.x, (0.5f - p.y * 0.5f) * VP_Size.y +VP_Min.y);
}
glm::vec3 EUI::ScreenToWorld(const ImVec2& pos) {
	ImVec2 p(-1.0f+2.0f*(pos.x-VP_Min.x)/VP_Size.x, 1.0f-2.0f*(pos.y - VP_Min.y) / VP_Size.y);
	glm::mat4 unproj = VP_ViewProjection;
	unproj[3] = glm::vec4(0,0,0,1);
	unproj = glm::inverse(unproj); //TODO: Cache Inverse
	return glm::vec3(unproj *glm::vec4(p.x, p.y, 1.0f, 0.0f));
}
void EUI::DrawLine(const glm::vec3& start, const glm::vec3& end) {
	auto a = WorldToScreen(start);
	auto b = WorldToScreen(end);
	VP_DrawList->AddLine(a, b, ImColor(1.0f, 0.0f, 0.0f));
}
void EUI::DrawVoxBounds(const glm::mat4& matrix, const glm::ivec3& size) {

	glm::vec3 pos = matrix[3];
	glm::vec3 dx = matrix[0];
	glm::vec3 dy = matrix[1];
	glm::vec3 dz = matrix[2];

	dx *= size.x * 0.1f;
	dy *= size.y * 0.1f;
	dz *= size.z * 0.1f;

	glm::vec3 v000 = pos;
	glm::vec3 v001 = pos + dz;
	glm::vec3 v010 = pos + dy;
	glm::vec3 v011 = pos + dy+dz;
	glm::vec3 v100 = pos + dx;
	glm::vec3 v101 = pos + dx+dz;
	glm::vec3 v110 = pos + dx+dy;
	glm::vec3 v111 = pos + dx+dy+dz;
	
	//Bottom
	DrawLine(v000, v100);
	DrawLine(v100, v101);
	DrawLine(v101, v001);
	DrawLine(v001, v000);

	//Sides
	DrawLine(v000, v010);
	DrawLine(v001, v011);
	DrawLine(v100, v110);
	DrawLine(v101, v111);

	//Top
	DrawLine(v010, v110);
	DrawLine(v110, v111);
	DrawLine(v111, v011);
	DrawLine(v011, v010);
	

	//VP_DrawList->AddCircleFilled(WorldToScreen(position+glm::vec3(size)*0.05f), 8.0, ImColor(0.0f, 1.0f, 0.0f));
}

void EUI::DrawSphere(const glm::vec3 pos, const float radius) {
	glm::vec4 p = VP_ViewProjection * glm::vec4(pos, 1.0f);
	if (p.w <= 0.0f) {
		p.w = 0.0000001f;
	}
	p.x /= p.w;
	p.y /= p.w;
	ImVec2 sp = ImVec2((p.x * 0.5f + 0.5f) * VP_Size.x + VP_Min.x, (0.5f - p.y * 0.5f) * VP_Size.y + VP_Min.y);
	
	VP_DrawList->AddCircleFilled(sp, radius*100.0f / p.w, ImColor(0.0f, 1.0f, 0.0f));
}

//Entity Picker

static bool picker_is_active;
static ImVec2 picker_line_start;
static World* picker_world;
static entt::entity picker_entity;
static std::string picker_data;
static ImGuiID picker_active_id;

std::string EUI::EntityPickerSource(ImVec2 pos) {
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
		picker_is_active = true;
		picker_line_start = pos;
		picker_world = nullptr;
		picker_entity = entt::null;
		picker_data = "";
		picker_active_id = ImGui::GetItemID();
	}

	if (picker_active_id == ImGui::GetItemID()) {
		if (picker_is_active) {
			ImGui::GetOverlayDrawList()->AddLine(picker_line_start, ImGui::GetMousePos(), ImGui::GetColorU32(ImGuiCol_Text));
		}

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			picker_is_active = false;
			if (picker_world != nullptr) {
				return picker_world->GetEntityPath(picker_entity);
			}
		}
	}
	return "";
}

void EUI::EntityPickerTarget(World* w, entt::entity e) {
	auto min = ImGui::GetItemRectMin();
	auto max = ImGui::GetItemRectMax();

	if (picker_is_active && ImGui::IsMouseHoveringRect(min, max)) {
		picker_world = w;
		picker_entity = e;

		ImGui::GetOverlayDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::GetColorU32(ImGuiCol_Text), 2);
	} 
}

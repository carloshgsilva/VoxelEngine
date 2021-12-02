#include "EditorSelection.h"

#include "Core/Core.h"

#include <entt/entt.hpp>

entt::entity EditorSelection::GetEntity() { return _Entities.size() > 0 ? _Entities[_Entities.size() - 1] : entt::null; }

bool EditorSelection::IsSelected(entt::entity e) {
	for (auto& se : _Entities) {
		if (se == e) {
			return true;
		}
	}
	return false;
}



void EditorSelection::AddEntity(entt::entity e) {
	if (IsSelected(e))RemoveEntity(e);
	_Entities.push_back(e);
}

void EditorSelection::RemoveEntity(entt::entity e) {
	CHECK(IsSelected(e));
	_Entities.erase(std::find(_Entities.begin(), _Entities.end(), e));
}

void EditorSelection::ToggleEntity(entt::entity e) {
	if (IsSelected(e)) {
		RemoveEntity(e);
	}
	else {
		AddEntity(e);
	}
}

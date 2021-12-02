#pragma once

#include <vector>
#include <entt/entity/fwd.hpp>

class EditorSelection {
	class World* _World;
	std::vector<entt::entity> _Entities;
public:
	World* GetWorld() { return _World; }
	void SetWorld(class World* W) { _World = W; }

	//Query Operations
	entt::entity GetEntity();
	const std::vector<entt::entity>& GetEntities() { return _Entities; }
	bool IsSelected(entt::entity e);
	bool HasSelection() { return _Entities.size() > 0; }

	//Edit Operations
	void Clear() { _Entities.clear(); }
	void AddEntity(entt::entity e);
	void RemoveEntity(entt::entity e);
	void ToggleEntity(entt::entity e);
};
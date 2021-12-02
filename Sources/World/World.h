#pragma once

#include "Systems/System.h"
#include "Asset/PrefabAsset.h"

#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <vector>

//TODO: replace entt::entity with Entity 
//using Entity = entt::entity;
//inline constexpr Entity EntityNull = Entity{};

class World {
	entt::registry _Registry;
	entt::entity _Root;
	AssetRefT<PrefabAsset> _Level;

	bool _isSimulating;

public:

	//Systems
	class TransformSystem* Transform;
	class PhysicsSystem* Physics;
	class ShadowVoxSystem* ShadowVox;
	class ScriptSystem* Script;
	class CharacterSystem* Character;
	class IKSystem* IK;

	World();
	~World();

	void Update(DeltaTime dt);

	void SetSimulate(bool value) { _isSimulating = value; }
	bool IsSimulating() { return _isSimulating; }

	template<typename T, typename = std::enable_if<std::is_base_of_v<System, T>>>
	T* RegisterSystem() {
		T* newSystem = new T();
		newSystem->R = &GetRegistry();
		newSystem->W = this;
		newSystem->OnCreate();
		return newSystem;
	}

	// Creates a new entity.
	// 
	// The entity name can't be duplicated in the same parent. If it occurs the name will change
	entt::entity Create(const std::string& name = "");
	entt::entity GetParent(entt::entity e);
	void SetParent(entt::entity child, entt::entity parent);
	const std::string& GetName(entt::entity e);
	void SetName(entt::entity e, const std::string& name);
	void Destroy(entt::entity e);
	void ForEachChild(entt::entity e, std::function<void(entt::entity child)> cb);

	// Find the nearest entity with Instance component, searching from bottom to top
	entt::entity GetInstanceScope(entt::entity e);

	// Get path relative to the nearest Instance entity
	// If not found, the World root will be used
	// TODO: Remove scope as we will deduce it from the Instance component
	std::string GetEntityPath(entt::entity e);

	// Find the child with a name
	// Finds it relative to the nearest Instance entity or World root if none found
	// returns the entity if succeeded or entt::null if not
	entt::entity FindChild(const std::string_view& name, entt::entity parent);

	// Find the child by following the path
	// Eg: "parent/child/grandchild"
	// if ctx is null, the search will be relative to World root
	// returns the entity if succeeded or entt::null if not
	entt::entity FindEntity(const std::string& path, entt::entity ctx = entt::null);

	bool IsSuperParent(entt::entity child, entt::entity superParent);

	entt::entity GetRoot() {
		return _Root;
	}
	AssetRefT<PrefabAsset>& GetLevel() {
		return _Level;
	}

	//Set the World Root to this Level
	//Any created entity will be child of the root
	//Is also the edited Level/Prefab
	void SetLevel(AssetRefT<PrefabAsset>& prefab) {
		_Registry.clear();
		_Level = prefab;
		_Root = prefab->Spawn(this);
	}

	entt::registry& GetRegistry() { return _Registry; }
};
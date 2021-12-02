#pragma once

#include "Asset/PalleteAsset.h"
#include "Asset/VoxAsset.h"
#include "Asset/PrefabAsset.h"
#include "Asset/ScriptAsset.h"
#include "Script/ScriptVM.h"
#include "World/EntitySlot.h"

#include <vector>
#include <entt/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct Hierarchy {
	std::string Name;
	entt::entity Parent{ entt::null };
	std::vector<entt::entity> Children{};
};

struct Script {
	AssetSlot<ScriptAsset> Asset;
	Handle Obj;
};

struct Instance {
	AssetSlot<PrefabAsset> Prefab;
};

struct VoxRenderer {
	AssetSlot<PalleteAsset> Pallete;
	AssetSlot<VoxAsset> Vox;
	int VoxSlot{-1};
	glm::vec3 Pivot{ 0.0f,0.0f,0.0f };
};

struct Light {

	enum class Type {
		Point,
		Spot,
		Directional,
		Ambient
	};

	Type LightType{ Type::Point };
	float Intensity{ 2.0f };
	glm::vec3 Color{ 1.0f,1.0f,1.0f };

	//Used in Point and Spot Lights
	float Attenuation{ 2.0f };
	float Range{ 10.0f };

	//Used in Spot Lights
	float Angle{ 0.3f };
	float AngleAttenuation{ 1.0f };
};


struct Transform {
	glm::mat4 PreviousWorldMatrix{ 1.0f };
	glm::mat4 WorldMatrix{ 1.0f };
	glm::mat4 Matrix{ 1.0f };
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f };
	glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };
};

struct Camera {
	float Fov{ 0.8f };
	float Near{ 0.1f };
	float Far{ 512.0f };
};

// Contains the state for a character
// The character movement and collision
// The procedural animation (head, arms, and legs)
struct Character {
	glm::vec3 Velocity{ 0.0f,0.0f,0.0f };
	glm::vec3 Rotation{ 0.0f,0.0f,0.0f };

	//IKs
	entt::entity HeadIK{ entt::null };
	entt::entity BodyIK{ entt::null };
	entt::entity LeftFootIK{ entt::null };
	entt::entity RightFootIK{ entt::null };
	entt::entity LeftHandIK{ entt::null };
	entt::entity RightHandIK{ entt::null };

	bool Running{ false };
	bool IsInAir{ true };
	float AirTime{ 0.0f };
	glm::vec3 LeftFootA{ 0.0f,0.0f,0.0f };
	glm::vec3 LeftFootB{ 0.0f,0.0f,0.0f };
	glm::vec3 RightFootA{ 0.0f,0.0f,0.0f };
	glm::vec3 RightFootB{ 0.0f,0.0f,0.0f };
	float IsRightFootUp{ true };
	float FootAirTime{ 0.0f };
};

// There should be only one Network Component per Hierarchy
// Scripts will search bottom top for Network rpc commands
// Set an callback in the NetworkingSystem to handle per NetworkComponent rpc
struct Networked {
	unsigned int ID;
	//Asset<Prefab> Prefab;
	//TODO: Network Sync Type
	//1 - Script Sync
	//2 - Player Sync
	//3 - ...
};


struct IKChain {
	EntitySlot Target;
	EntitySlot Pole;
	int Depth{ 3 };
	std::vector<entt::entity> Bones;
	std::vector<glm::vec3> Positions;
	std::vector<float> Lengths;
};
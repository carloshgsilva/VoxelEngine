#pragma once

#include <string>
#include "World.h"
#include <entt/fwd.hpp>


class EntitySlot {
	std::string Path;
	entt::entity Entity;
	bool Loaded{ false };

public:
	EntitySlot();
	EntitySlot(const std::string& path);

	// Get an entity by using the stored path in the EntitySlot.
	// src entity used as context
	// returns the entity if found or entt::null if not found
	entt::entity GetEntity(World* w, entt::entity ctx);

	const std::string& GetPath() { return Path; }
	const void SetPath(const std::string& path) { Path = path; Loaded = false; }
};
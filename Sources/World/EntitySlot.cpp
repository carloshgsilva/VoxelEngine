#include "EntitySlot.h"

#include <entt/entt.hpp>

EntitySlot::EntitySlot() : Path(""), Entity(entt::null) {}

EntitySlot::EntitySlot(const std::string& path) : Path(path), Entity(entt::null) {
}

entt::entity EntitySlot::GetEntity(World* w, entt::entity ctx) {
	if (!Loaded) {
		Entity = w->FindEntity(Path, ctx);
		Loaded = true;
	}
	return Entity;
}

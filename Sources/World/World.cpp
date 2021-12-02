#include "World.h"

#include "World/Components.h"

#include "Systems/TransformSystem.h"
#include "Systems/PhysicsSystem.h"
#include "Systems/ShadowVoxSystem.h"
#include "Systems/ScriptSystem.h"
#include "Systems/CharacterSystem.h"
#include "Systems/IKSystem.h"
#include "Profiler/Profiler.h"

World::World() {
	Transform = RegisterSystem<TransformSystem>();
	Physics = RegisterSystem<PhysicsSystem>();
	ShadowVox = RegisterSystem<ShadowVoxSystem>();
	Script = RegisterSystem<ScriptSystem>();
	Character = RegisterSystem<CharacterSystem>();
	IK = RegisterSystem<IKSystem>();

	_Root = Create();
}

World::~World() {
	delete Transform;
	delete Physics;
	delete ShadowVox;
	delete Script;
	delete Character;
	delete IK;
}

void World::Update(DeltaTime dt) {
	PROFILE_FUNC();

	if (_isSimulating) {
		Script->OnUpdate(dt);
		Physics->OnUpdate(dt);
	}

	Character->OnUpdate(dt);
	Transform->OnUpdate(dt);
	IK->OnUpdate(dt);
	ShadowVox->OnUpdate(dt);
}

entt::entity World::Create(const std::string& name) {
	auto e = _Registry.create();
	_Registry.emplace<Hierarchy>(e);
	return e;
}

entt::entity World::GetParent(entt::entity e) {
	if (_Registry.has<Hierarchy>(e)) {
		return _Registry.get<Hierarchy>(e).Parent;
	}
	return entt::null;
}
void World::SetParent(entt::entity child, entt::entity parent) {
	if (!_Registry.has<Hierarchy>(child))_Registry.emplace<Hierarchy>(child);

	Hierarchy& h = _Registry.get<Hierarchy>(child);

	//Remove child from parent children
	if (h.Parent != entt::null) {
		Hierarchy& parent_h = _Registry.get<Hierarchy>(h.Parent);
		auto& parent_children = parent_h.Children;
		parent_children.erase(std::find(parent_children.begin(), parent_children.end(), child));
	}

	h.Parent = parent;

	//Add child to parent children
	if (parent != entt::null) {
		if (!_Registry.has<Hierarchy>(parent))_Registry.emplace<Hierarchy>(parent);
		_Registry.get<Hierarchy>(parent).Children.push_back(child);
	}
}

const std::string& World::GetName(entt::entity e) { return _Registry.get<Hierarchy>(e).Name; }
void World::SetName(entt::entity e, const std::string& name) { _Registry.get<Hierarchy>(e).Name = name; }

void destroyEntityRecursive(World* W, entt::entity e) {
	W->ForEachChild(e, [&](entt::entity e) {
		destroyEntityRecursive(W, e);
	});
	W->GetRegistry().destroy(e);
}

void World::Destroy(entt::entity e) {
	SetParent(e, entt::null);
	destroyEntityRecursive(this, e);
}

void World::ForEachChild(entt::entity e, std::function<void(entt::entity child)> cb) {
	entt::entity child = entt::null;
	Hierarchy& h = _Registry.get<Hierarchy>(e);
	for (auto e : h.Children) {
		cb(e);
	}
}

entt::entity World::GetInstanceScope(entt::entity e) {
	do {
		std::string& name = _Registry.get<Hierarchy>(e).Name;
		if (_Registry.has<Instance>(e)) {
			return e;
		}
		else {
			e = GetParent(e);
		}
	} while (e != entt::null);
	return GetRoot();
}

std::string World::GetEntityPath(entt::entity e) {
	entt::entity scope = GetInstanceScope(e);
	
	std::vector<entt::entity> hierarchy;
	entt::entity parent = e;
	while (parent != entt::null && parent != scope) {
		hierarchy.push_back(parent);
		parent = _Registry.has<Hierarchy>(parent) ? _Registry.get<Hierarchy>(parent).Parent : entt::null;
	}
	std::string path = "";
	for (int i = hierarchy.size() - 1; i >= 0; i--) {
		std::string& name = _Registry.get<Hierarchy>(hierarchy[i]).Name;
		if(i<hierarchy.size()-1)path += "/";
		path += name;
	}
	if (hierarchy.size() == 0 && parent == scope) {
		return ".";
	}
	return path;
}

entt::entity World::FindChild(const std::string_view& name, entt::entity parent) {
	if (!_Registry.valid(parent)) { return entt::null; }
	
	Hierarchy& h = _Registry.get<Hierarchy>(parent);
	for (auto child : h.Children) {
		if (_Registry.get<Hierarchy>(child).Name == name) {
			return child;
		}
	}

	return entt::null;
}

entt::entity World::FindEntity(const std::string& path, entt::entity ctx) {
	if (path == ".") {
		return GetRoot();
	}
	if (ctx == entt::null) {
		ctx = GetRoot();
	}
	
	ctx = GetInstanceScope(ctx);
	std::string_view view = path;

	int begin = 0;
	int end = 0;

	while (end != -1) {
		end = path.find_first_of('/', begin);
		std::string_view name = view.substr(begin, end - begin);
		entt::entity child = FindChild(name, ctx);

		if (child == entt::null) {
			return entt::null;
		}
		else {
			ctx = child;
		}
		begin = end+1;
	}

	return ctx;
}

bool World::IsSuperParent(entt::entity child, entt::entity superParent) {
	if (child == superParent)return true;
	entt::entity parent = child;

	do {
		parent = _Registry.has<Hierarchy>(parent) ? _Registry.get<Hierarchy>(parent).Parent : entt::null;

		if (parent == superParent) {
			return true;
		}
	} while (parent != entt::null);

	return false;
}

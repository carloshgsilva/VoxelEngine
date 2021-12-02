#pragma once

#include "System.h"

#include "Script/ScriptVM.h"
#include "Asset/ScriptAsset.h"

//m_ for methods
//s_ for setters
//g_ for getters
//g_ for globals
//c_ for classes
//f_ for foreign classes
struct WorldCtx {
	class World* world;
	ScriptVM* vm;
	Handle m_new;
	Handle m_update;
	Handle s_e;
	Handle c_script;
	Handle f_entity;

	WorldCtx(World* W, ScriptVM* svm) {
		world = W;
		vm = svm;
		m_new = vm->GetMethod("new()");
		m_update = vm->GetMethod("update()");

		s_e = vm->GetMethod("e=(_)");

		vm->RunModule("Core");

		c_script = vm->GetVariable("Core", "Script");
		f_entity = vm->GetVariable("Core", "Entity");
	}
	
	Handle wrapEntity(entt::entity e){ return vm->NewForeign<entt::entity>(f_entity, e); }
	Handle createScript(AssetRefT<ScriptAsset>& asset){}
};

class ScriptSystem : public System {

	Unique<ScriptVM> vm;
	WorldCtx* ctx;

	std::vector<Handle> _Scripts;

	void OnScriptUpdated(entt::registry& r, entt::entity e);
public:

	virtual void OnCreate();
	virtual void OnUpdate(DeltaTime dt);
	virtual void OnEvent(Event& e) {}
	virtual void OnDestroy() {}
};
#include "ScriptSystem.h"

#include "World/Components.h"
#include "Script/ScriptLib_Core.h"
#include "Mod/ModLoader.h"

void ScriptSystem::OnScriptUpdated(entt::registry& r, entt::entity e) {

	auto& scriptAsset = r.get<Script>(e).Asset;
	if (!scriptAsset.IsValid()) {
		return;
	}

	std::string scriptPath = "Mods/";
	scriptPath += ModLoader::Get().GetAssetPath(scriptAsset->GetGUID());
	scriptPath = scriptPath.substr(0, scriptPath.find_last_of('.'));
	std::string scriptName = scriptPath.substr(scriptPath.find_last_of('/')+1);

	if (!vm->HasModule(scriptPath)) {
		vm->RunModule(scriptPath);
	}

	Handle c_rotatingCube = vm->GetVariable(scriptPath, scriptName);
	
	if(vm->Call(c_rotatingCube, ctx->m_new)){
		Handle script = vm->GetHandle();
		
		Handle newForeign = ctx->wrapEntity(e);
		if (vm->Call(script, ctx->s_e, newForeign)) {
			_Scripts.push_back(script);
		}
	}
	
}

void ScriptSystem::OnCreate() {
	{
		vm = NewUnique<ScriptVM>();

		ScriptLib_Core::Import(*vm);

		ctx = new WorldCtx(W, vm.get());
		vm->SetUserData(ctx);
	}

	R->on_construct<Script>().connect<&ScriptSystem::OnScriptUpdated>(this);
	R->on_update<Script>().connect<&ScriptSystem::OnScriptUpdated>(this);
}


void ScriptSystem::OnUpdate(DeltaTime dt) {
	for (auto& s : _Scripts) {
		vm->Call(s, ctx->m_update);
	}
}

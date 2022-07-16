#pragma once

#include "Core/Module.h"
#include "Mod/Mod.h"

#include <filesystem>

class ModLoader : public ModuleDef<ModLoader> {
	std::unordered_map<std::string, Mod*> _LoadedMods;

	void PrepareMod(std::string name, Mod* mod) {
		//TODO: load mod.wren
		_LoadedMods.emplace(name, mod);
	}

public:
	inline static std::string INVALID_ASSET_PATH = "INVALID_ASSET_PATH";

	ModLoader();

	~ModLoader();
	
	// Tries to load an asset across all mods, if fail returns an invalid AssetRef
	AssetRef TryToLoad(AssetGUID guid);
	const std::string& GetAssetPath(AssetGUID guid);
	void AddPath(const std::string& path);
	void RemovePath(const std::string& path);
};
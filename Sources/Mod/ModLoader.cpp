#include "ModLoader.h"

ModLoader::ModLoader() {
	CHECK(std::filesystem::exists(std::filesystem::path(Mod::MODS)));

	for (auto path : std::filesystem::directory_iterator(Mod::MODS)) {
		if (path.is_directory()) {
			std::string modName = path.path().lexically_relative(Mod::MODS).generic_string();
			Log::info("[Mod] Loaded: {}", modName);
			PrepareMod(modName, new ModFolder(modName, path.path().generic_string()));
		}
		else {
			std::string ext = path.path().extension().generic_string();
			if (ext == ".mod") {
				Log::error("[Mod] Could not Load {} because .mod files are not supported yet!", path.path().generic_string());
			}
		}
	}

	CHECK(_LoadedMods.find("default") != _LoadedMods.end());
}

ModLoader::~ModLoader() {
	for (auto& pair : _LoadedMods) {
		delete pair.second;
	}
}

// Tries to load an asset across all mods, if fail returns an invalid AssetRef

AssetRef ModLoader::TryToLoad(AssetGUID guid) {
	for (auto& pair : _LoadedMods) {
		if (pair.second->Has(guid)) {
			return pair.second->Load(guid);
		}
	}
	return AssetRef();
}

const std::string& ModLoader::GetAssetPath(AssetGUID guid) {
	for (auto& pair : _LoadedMods) {
		if (pair.second->Has(guid)) {
			return pair.second->GetAssetPath(guid);
		}
	}
	return INVALID_ASSET_PATH;
}

void ModLoader::AddPath(const std::string& path) {
	std::string modName = path.substr(0, path.find_first_of('/'));
	_LoadedMods[modName]->AddPath(path);
}

void ModLoader::RemovePath(const std::string& path) {
	std::string modName = path.substr(0, path.find_first_of('/'));
	_LoadedMods[modName]->RemovePath(path);

}

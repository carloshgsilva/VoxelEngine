#pragma once

#include "Asset/Assets.h"

class ModVersion {
	int Major;
	int Minor;
};


/* mod.wren
// This is a file that is in the root of the mod and contain informations about it
// You can add a new Gamemode

var Mod = {
	"name": "My Mod",
	"description": "It's a really cool mod!",
	"version": "1.0",
	"dependencies": [
		"modname1",
		"modname2",
	]
}

AddGamemode();

*/

class Mod {
	std::string _Name;
	//std::vector<std::string> _Dependencies;
	std::unordered_map<AssetGUID, std::string> _AssetsMap;

protected:
	void ClearCache() {
		_AssetsMap.clear();
	}
	void Cache(const std::string& path) {
		_AssetsMap.emplace(Assets::Hash(path), path);
	}


	virtual AssetRef Load(const std::string& path) { CHECK(0); return {}; }

public:
	inline static const std::filesystem::path MODS = "Assets/Mods";

	Mod(std::string name) : _Name(name) {

	}

	virtual void AddPath(const std::string& path) { CHECK(0); }
	void RemovePath(const std::string& path) {
		AssetGUID guid = Assets::Hash(path);
		assert(_AssetsMap.find(guid) != _AssetsMap.end());

		_AssetsMap.erase(guid);
	}

	bool Has(AssetGUID guid) {
		return _AssetsMap.find(guid) != _AssetsMap.end();
	}
	const std::string& GetName(AssetGUID guid) {
		CHECK(Has(guid));
		return _AssetsMap[guid];
	}
	AssetRef Load(AssetGUID guid) {
		CHECK(Has(guid));
		return Load(_AssetsMap[guid]);
	}
	const std::string& GetAssetPath(AssetGUID guid) {
		return _AssetsMap[guid];
	}
};

//TODO: class ModPackage : public Mod {};

class ModFolder : public Mod {
	std::string _ModName;
	std::string _Path;

	virtual AssetRef Load(const std::string& path) {
		FileReader fr((MODS/path).generic_string());
		return AssetSerializer::Load(fr);
	}

	void _MapModFolder() {
		ClearCache();
		for (auto path : std::filesystem::recursive_directory_iterator(_Path)) {
			if (!path.is_directory()) {
				std::string identifier = path.path().lexically_relative(MODS).generic_string();
				//Log::info("[Mod] [{}] {:X} {}", _ModName, Assets::Hash(identifier), identifier);
				Cache(identifier);
			}
		}
	}

public:
	ModFolder(const std::string& name, const std::string& folderPath) : _ModName(name), _Path(folderPath), Mod(name) {
		_MapModFolder();
	}

	// Should be called when editor create a new asset
	virtual void AddPath(const std::string& path) {
		auto filePath = MODS / path;
		assert(std::filesystem::exists(filePath));
		Cache(path);
	}

};
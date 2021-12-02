#include "Assets.h"

#include "Mod/ModLoader.h"

AssetRef Assets::_Load(AssetGUID guid) {
	//Asset not loaded load from file
	if (_AssetsCache.find(guid) == _AssetsCache.end()) {

		AssetRef asset = ModLoader::Get().TryToLoad(guid);

		if (asset.IsValid()) {
			asset->_GUID = guid;
			_AssetsCache.emplace(guid, asset);
		}
		
		return asset;
	}
	//Asset already loaded use the cached
	else {
		return _AssetsCache[guid];
	}
}

void Assets::CreateAsset(AssetRef asset, const std::string& path) {
	//TODO: Check path validity
	std::string ensurePath = (Mod::MODS / path).parent_path().generic_string();
	std::filesystem::create_directory(ensurePath);

	std::string relPath = (Mod::MODS / path).generic_string();

	CHECK(asset->_GUID == Asset::NullGUID);

	asset->_GUID = Hash(path);

	{
		FileWriter fr(relPath);
		asset->Serialize(fr);
	}

	Get()._AssetsCache[asset->_GUID] = asset;
	ModLoader::Get().AddPath(path);
}

void Assets::SaveAsset(AssetRef asset) {
	CHECK(asset->_GUID != Asset::NullGUID); // Asset must already exist in a file, use Assets::CreateAsset instead!
	
	//TODO: Check path validity
	std::string path = ModLoader::Get().GetAssetPath(asset->GetGUID());
	std::string ensurePath = (std::filesystem::path("Mods") / std::filesystem::path(path)).parent_path().generic_string();
	std::filesystem::create_directory(ensurePath);

	std::string relPath = fmt::format("Mods/{}", path);
	
	CHECK(std::filesystem::exists(relPath)); // Ensures the file is created before writing to it
	Log::info("Saving {}", relPath);

	FileWriter fr(relPath);
	asset->Serialize(fr);
}

void Assets::DeleteAsset(AssetGUID guid) {
	//Null Asset GUID if already loaded in memory
	if (Get()._AssetsCache.find(guid) != Get()._AssetsCache.end()) {
		Get()._AssetsCache[guid]->_GUID = Asset::NullGUID;
		Get()._AssetsCache.erase(guid);
	}

	auto path = ModLoader::Get().GetAssetPath(guid);
	auto absPath = Mod::MODS / path;

	std::filesystem::remove(absPath);
	ModLoader::Get().RemovePath(path);
}

#include "AutoAssetImporter.h"

#include "Mod/ModLoader.h"
#include "Editor/Importer/AssetImporter.h"

#include <filesystem>
#include "AutoAssetImporter.h"

AutoAssetImporter::AutoAssetImporter() {
	lastRootEdit = std::filesystem::last_write_time(Mod::MODS).time_since_epoch().count();

	for (auto path : std::filesystem::recursive_directory_iterator(Mod::MODS)) {
		std::string filePath = path.path().generic_string();
		if (AssetImporter::CanImport(filePath)) {
			filesLastEdit.emplace(filePath, std::filesystem::last_write_time(path).time_since_epoch().count());
		}
	}
}

void AutoAssetImporter::CheckForImport() {
	long long newRootEdit = std::filesystem::last_write_time(Mod::MODS).time_since_epoch().count();
	if (Get().lastRootEdit == newRootEdit) {
		return;
	}
	Get().lastRootEdit = newRootEdit;

	for (auto path : std::filesystem::recursive_directory_iterator(Mod::MODS)) {
		std::string filePath = path.path().generic_string();
		if (AssetImporter::CanImport(filePath)) {
			long long lastEdit = std::filesystem::last_write_time(path).time_since_epoch().count();
			if (Get().filesLastEdit[filePath] != lastEdit) {
				Get().filesLastEdit[filePath] = lastEdit;
				Log::info("Auto import {} to {}", filePath, path.path().parent_path().lexically_relative(Mod::MODS).generic_string());
				AssetImporter::Import(filePath, path.path().parent_path().lexically_relative(Mod::MODS).generic_string());
			}
		}
	}
}

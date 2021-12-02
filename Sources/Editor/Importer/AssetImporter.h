#pragma once

#include "Core/Engine.h"
#include "Asset/Assets.h"
#include "IO/Stream.h"

#include <unordered_map>
#include <string>
#include <filesystem>

// Extended by any that that implements an importer for files (.vox, .png, ...)
// Represents a file format importer

class AssetImporter {

	using ImporterFactory = std::function<void(Stream&, std::string, std::string)>;
	using TImporterMap = std::unordered_map<std::string, ImporterFactory>;
	static TImporterMap& Importers() {
		static TImporterMap map;
		return map;
	}

protected:
	template<typename T>
	static bool Register(std::string type) {
		Importers()[type] = [](Stream& s, std::string file, std::string path) {
			T importer = {};
			importer._FileName = file;
			importer._Path = path;
			importer.Import(s);
		};
		return true;
	}

	static void SaveAsset(AssetRef asset, std::string filePath) {
		FileWriter fw(filePath);
		AssetSerializer::Save(asset, fw);
	}

	std::filesystem::path _Path;
	std::filesystem::path _FileName;

public:

	virtual void Import(Stream& s) { CHECK(0); }

	static bool CanImport(std::string file) {
		std::filesystem::path p = file;
		if (!p.has_extension())return false;
		std::string ext = p.extension().string().substr(1);
		return Importers().find(ext) != Importers().end();
	}

	//Import a file to a folder
	//returns true if succeded and false if failed
	static bool Import(std::string file, std::string to) {
		std::filesystem::path p = file;
		std::string ext = p.extension().string().substr(1);
		FileReader fr(file);

		if (Importers().find(ext) != Importers().end()) { //Must Use CanImport before using it
			Importers().at(ext)(fr, p.stem().generic_string(), to);
			AssetImportedEvent e;
			e.file = file;
			e.toFolder = to;
			Engine::Get().DispatchEvent(e);

			return true;
		}
		else {
			Log::error("Tried to import {} to {} but the import of type '{}' was not found!", file, to, p.extension().generic_string());
			return false;
		}
	}

};

#include "FileExplorer.h"

#include "Core/Core.h"
#include "Mod/ModLoader.h"

static void FillFolderFiles(DirEntry& entry) {
	CHECK(entry.isFolder);
	entry.files.clear();
	for (auto& file : std::filesystem::directory_iterator(entry.Path)) {
		entry.files.push_back(file.path().generic_string());
	}
	std::sort(entry.files.begin(), entry.files.end());
}

FileExplorer::FileExplorer(std::string folder) {
	SetFolder(folder);
}

void FileExplorer::SetFolder(std::string folder) {
	DirEntry entry;
	entry.Path = std::filesystem::path(folder).generic_string();
	entry.isFolder = true;
	entry.Name = std::filesystem::path(folder).filename().generic_string();

	_Root = entry;
	_Invalid = true;
	Refresh();
}

void FileExplorer::Refresh() {
	if (_Root.Path.size() == 0 || !_Invalid)return;
	_FilesCache.clear();
	
	//Recursive Files Mapping
	for(auto& file : std::filesystem::recursive_directory_iterator(_Root.Path)){
		DirEntry f;
		f.isFolder = file.is_directory();
		f.RelativePath = file.path().lexically_relative(Mod::MODS).generic_string();
		f.Path = file.path().generic_string();
		f.Name = file.path().filename().generic_string();

		_FilesCache.emplace(f.Path, f);
	}

	//Get Children for root
	FillFolderFiles(_Root);

	//Get Children of each folder
	for (auto& cache : _FilesCache) {
		DirEntry& e = cache.second;
		if (e.isFolder) {
			FillFolderFiles(e);
		}
	}

	_FilesCache.emplace(_Root.Path, _Root);

	_Invalid = false;
}


void FileExplorer::Explore(DirEntry& entry, std::function<void(DirEntry&)> _Cb) {
	CHECK(entry.isFolder);

	for (auto& childEntry : entry.files) {
		_Cb(_FilesCache[childEntry]);
	}
}
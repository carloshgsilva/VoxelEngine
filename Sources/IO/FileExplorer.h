#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <functional>

struct DirEntry {
	std::string RelativePath;
	std::string Path;
	std::string Name;
	bool isFolder;
	std::vector<std::string> files;
};

class FileExplorer {
	DirEntry _Root;
	std::unordered_map<std::string, DirEntry> _FilesCache;
	bool _Invalid;

public:
	FileExplorer(){}
	FileExplorer(std::string folder);

	void SetFolder(std::string folder);
	DirEntry& GetRoot() { return _Root; }
	DirEntry& GetDir(const std::string& path) { return _FilesCache[path]; }

	bool HasChildFolder(DirEntry& entry) {
		for (auto& str : entry.files) {
			if (_FilesCache[str].isFolder) {
				return true;
			}
		}
		return false;
	}

	//Will invalidate and on the Refresh call the directory will be scanned again
	void Invalidate() { _Invalid = true; }

	//Should be called every frame
	void Refresh();
	void Explore(DirEntry& entry, std::function<void(DirEntry&)> _Cb);
};
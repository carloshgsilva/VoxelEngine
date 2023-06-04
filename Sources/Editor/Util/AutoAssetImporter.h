#pragma once

#include <unordered_map>
#include <string>

class AutoAssetImporter {
	long long lastRootEdit;

	std::unordered_map<std::string, long long> filesLastEdit = {};

	static AutoAssetImporter& Get() {
		static AutoAssetImporter i;
		return i;
	}

public:
	AutoAssetImporter();

	static void CheckForImport();
};
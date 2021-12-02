#pragma once

#include "Core/Core.h"
#include "IO/Stream.h"

#include <vector>
#include <string>

class FileUtil {
public:
	static std::vector<uint8> ReadBytes(std::string FilePath) {
		std::vector<uint8> data;
		FileReader fr(FilePath);
		data.resize(fr.GetSize());
		fr.Serialize(data.data(), fr.GetSize());
		return data;
	}
};
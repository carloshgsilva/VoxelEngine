#pragma once

#include "AssetImporter.h"

class VoxImporter : public AssetImporter {
	inline static const bool Registered = Register<VoxImporter>("vox");

public:

	virtual void Import(Stream& s);
};

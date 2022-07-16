#pragma once

#include "Assets.h"

class ScriptAsset : public Asset {
	ASSET(ScriptAsset, wren)

	std::string _Src;

public:

	std::string& GetSrc() { return _Src; }

	virtual void Serialize(Stream& S) {
		if (S.IsLoading()) {
			_Src.resize(S.GetSize());
		}
		S.Serialize(_Src.data(), _Src.size());
	}
};
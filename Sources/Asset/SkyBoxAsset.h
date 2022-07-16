#pragma once

#include "Assets.h"
#include "Graphics/Graphics.h"

class SkyBoxAsset : public Asset {
	ASSET(SkyBoxAsset, hdr)

	Image _Image;

	virtual void Serialize(Stream& S);

public:
	Image& GetSkyBox() { return _Image; }

};
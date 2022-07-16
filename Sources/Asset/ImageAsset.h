#pragma once

#include "Assets.h"
#include "Graphics/Graphics.h"

class ImageAsset : public Asset {
	ASSET(ImageAsset, png)
	
	Image _Image;

	virtual void Serialize(Stream& S);
public:
	Image& GetImage() { return _Image; }
};
#pragma once

#include "System.h"

#include "Graphics/Graphics.h"

class ShadowVoxSystem : public System {
	Buffer _Buffer;
	Image _Volume;
	std::vector<ImageRegion> _UpdateRegions;


	uint32 _SizeX;
	uint32 _SizeY;
	uint32 _SizeZ;
	uint32 _SizeXtimes_SizeY;

	std::vector<uint8> _Data;

	void OnVoxDestroyed(entt::registry& r, entt::entity e);

public:

	ShadowVoxSystem();

	inline void SetVolumeAt(int x, int y, int z, int value);
	inline bool GetVolumeAt(int x, int y, int z);

	Image& GetVolumeImage() { return _Volume; }

	virtual void OnCreate();
	virtual void OnUpdate(DeltaTime dt);
	virtual void OnEvent(Event& e) {}
	virtual void OnDestroy() {}
};
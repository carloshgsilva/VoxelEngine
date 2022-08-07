#pragma once

#include "Core/Core.h"

#include "World/World.h"

#include "Graphics/Renderer/View.h"
#include "Asset/ImageAsset.h"
#include "Asset/SkyBoxAsset.h"

#include <glm/vec3.hpp>

//Handles all the state required to Render a World
class WorldRenderer {

	View lastView = {};

	AssetRefT<ImageAsset> _BlueNoise;
	AssetRefT<SkyBoxAsset> _DefaultSkyBox;
	Buffer _ViewBuffer;

	int _Frame{ 0 };
	
	//Framebuffers
	GBuffer gbuffer;

	Image _LastLightBuffer;
	Image _CurrentLightBuffer;
	Image _TAALightBuffer;
	Image _ReflectionBuffer;

	Image _BloomStepBuffer;
	std::vector<Image> _Bloom1Buffer;
	std::vector<Image> _Bloom2Buffer;

	Image _OutlineBuffer;

	Image _LastComposeBuffer;
	Image _CurrentComposeBuffer;
	Image _TAAComposeBuffer;

	Image _ColorBuffer;

	struct RenderCmds* Cmds;

public:

	WorldRenderer();
	~WorldRenderer() {
		delete Cmds;
	}

	void CmdOutline(const glm::mat4& matrix, Image& vox, glm::vec3 color);
	void CmdVoxel(const glm::mat4& matrix, const glm::mat4& lastMatrix, Image& vox, int palleteIndex, int id = 0);

	void RecreateFramebuffer(uint32 Width, uint32 Height);

	void DrawWorld(float dt, View& view, World& world);

	Image& GetCurrentColorImage();
};
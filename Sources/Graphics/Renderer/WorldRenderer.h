#pragma once

#include "Core/Core.h"
#include "Core/DeltaTime.h"

#include "World/World.h"

#include "Graphics/Renderer/View.h"
#include "Asset/ImageAsset.h"
#include "Asset/SkyBoxAsset.h"

#include <glm/vec3.hpp>

//Handles all the state required to Render a World
class WorldRenderer {

	View _LastView;

	AssetRefT<ImageAsset> _BlueNoise;
	AssetRefT<SkyBoxAsset> _DefaultSkyBox;
	Buffer _ViewBuffer;

	int _Frame{ 0 };
	
	//Framebuffers
	Framebuffer _GeometryBuffer;

	Framebuffer _LastLightBuffer;
	Framebuffer _CurrentLightBuffer;
	Framebuffer _TAALightBuffer;
	Framebuffer _ReflectionBuffer;

	Framebuffer _BloomStepBuffer;
	std::vector<Framebuffer> _Bloom1Buffer;
	std::vector<Framebuffer> _Bloom2Buffer;

	Framebuffer _OutlineBuffer;

	Framebuffer _LastComposeBuffer;
	Framebuffer _CurrentComposeBuffer;
	Framebuffer _TAAComposeBuffer;

	Framebuffer _ColorBuffer;

	struct RenderCmds* Cmds;

public:

	WorldRenderer();
	~WorldRenderer() {
		delete Cmds;
	}

	void CmdOutline(const glm::mat4& matrix, Image& vox, glm::vec3 color);
	void CmdVoxel(const glm::mat4& matrix, const glm::mat4& lastMatrix, Image& vox, int palleteIndex, int id = 0);

	void RecreateFramebuffer(int32 Width, int32 Height);

	void DrawWorld(DeltaTime dt, CmdBuffer& cmd, View& view, World& world);

	Image& GetCurrentColorImage();
};
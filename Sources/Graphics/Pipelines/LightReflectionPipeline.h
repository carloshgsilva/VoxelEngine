#pragma once


#include "Graphics/Graphics.h"
#include "glm/glm.hpp"
#include "Util/FileUtil.h"
#include <functional>
#include "Core/Engine.h"

#include "Graphics/Renderer/View.h"

class LightReflectionPipeline {
	struct PushConstant {
		int ViewBufferRID;
		int LightTextureRID;
		int NormalTextureRID;
		int MaterialTextureRID;
		int DepthTextureRID;
		int BlueNoiseTextureRID;
		int SkyBoxTextureRID;
		int ShadowVoxRID;
	};

	Pipeline pipeline;
public:
	LightReflectionPipeline() {
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightReflection.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightReflection.frag.spv"),
			.attachments = {Format::RGBA16Sfloat},
		});
	}

	void Use(Buffer& viewBuffer, Image& lightDiffuse, GBuffer& gbuffer, Image& shadowVox, Image& skybox, Image& blueNoise) {
		//Update Push Constances
		PushConstant pc = {};
		pc.ViewBufferRID = GetRID(viewBuffer);
		pc.LightTextureRID = GetRID(lightDiffuse);
		pc.NormalTextureRID = GetRID(gbuffer.normal);
		pc.MaterialTextureRID = GetRID(gbuffer.material);
		pc.DepthTextureRID = GetRID(gbuffer.depth);
		pc.BlueNoiseTextureRID = GetRID(blueNoise);
		pc.ShadowVoxRID = GetRID(shadowVox);
		pc.SkyBoxTextureRID = GetRID(skybox);

		//TODO: Fill with
		CmdPush(pc);

		CmdBind(pipeline);
		CmdDraw(6, 1, 0, 0);
	}

	static LightReflectionPipeline& Get() {
		static LightReflectionPipeline Instance;
		return Instance;
	}

};
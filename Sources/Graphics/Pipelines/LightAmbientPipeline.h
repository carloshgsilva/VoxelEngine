#pragma once


#include "Graphics/Graphics.h"
#include "glm/glm.hpp"
#include "Util/FileUtil.h"
#include <functional>
#include "Core/Engine.h"

#include "Graphics/Renderer/View.h"

class LightAmbientPipeline {
	struct PushConstant {
		int ViewBufferRID;
		int ColorTextureRID;
		int NormalTextureRID;
		int MaterialTextureRID;
		int DepthTextureRID;
		int BlueNoiseTextureRID;
		int ShadowVoxRID;
		int SkyBoxRID;
	};

	Pipeline pipeline;
public:
	LightAmbientPipeline() {
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightAmbient.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightAmbient.frag.spv"),
			.attachments = {Format::RGBA16Sfloat},
			.blends = {Blend::Additive}
		});
	}

	void Use(Buffer& viewBuffer, GBuffer& gbuffer, Image& shadowVox, Image& blueNoise, Image& skyBox) {
		//Update Push Constances
		PushConstant pc = {};
		pc.ViewBufferRID = GetRID(viewBuffer);
		pc.ColorTextureRID = GetRID(gbuffer.color);
		pc.NormalTextureRID = GetRID(gbuffer.normal);
		pc.MaterialTextureRID = GetRID(gbuffer.material);
		pc.DepthTextureRID = GetRID(gbuffer.depth);
		pc.BlueNoiseTextureRID = GetRID(blueNoise);
		pc.ShadowVoxRID = GetRID(shadowVox);
		pc.SkyBoxRID = GetRID(skyBox);

		//TODO: Fill with
		CmdPush(pc);

		CmdBind(pipeline);
		CmdDraw(6, 1, 0, 0);
	}

	static LightAmbientPipeline& Get() {
		static LightAmbientPipeline Instance;
		return Instance;
	}

};
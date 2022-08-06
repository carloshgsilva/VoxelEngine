#pragma once


#include "Graphics/Graphics.h"
#include "glm/glm.hpp"
#include <functional>

#include "Graphics/Renderer/View.h"

class LightTAAPipeline {
	Pipeline pipeline;

	struct PushConstant {
		int ViewBufferRID;
		int BlueNoiseTextureRID;
		int MotionTextureRID;
		int LightTextureRID;
		int ColorTextureRID;
		int NormalTextureRID;
		int MaterialTextureRID;
		int DepthTextureRID;
		int LastLightTextureRID;
	};

public:
	LightTAAPipeline() {
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightTAA.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightTAA.frag.spv"),
			.attachments = {Format::RGBA16Sfloat}
		});
	}

	void Use(Buffer& viewbuffer, Image& blueNoise, Image& lastLightTAA, Image& light, GBuffer& gbuffer) {
		
		PushConstant pc;
		pc.ViewBufferRID = GetRID(viewbuffer);
		pc.BlueNoiseTextureRID = GetRID(blueNoise);
		pc.MotionTextureRID = GetRID(gbuffer.motion);
		pc.LightTextureRID = GetRID(light);
		pc.ColorTextureRID = GetRID(gbuffer.color);
		pc.NormalTextureRID = GetRID(gbuffer.normal);
		pc.MaterialTextureRID = GetRID(gbuffer.material);
		pc.DepthTextureRID = GetRID(gbuffer.depth);
		pc.LastLightTextureRID = GetRID(lastLightTAA);
		
		CmdBind(pipeline);
		CmdPush(pc);
		CmdDraw(6, 1, 0, 0);
		
	}

	static LightTAAPipeline& Get() {
		static LightTAAPipeline Instance;
		return Instance;
	}

};
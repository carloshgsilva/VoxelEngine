#pragma once


#include "Graphics/Graphics.h"
#include "glm/glm.hpp"
#include "Util/FileUtil.h"
#include <functional>

#include "Graphics/Renderer/View.h"

class ComposeTAAPipeline {

	struct PushConstant {
		int ViewBufferRID;
		int MotionTextureRID;
		int LastColorTextureRID;
		int ColorTextureRID;
	};

	Pipeline pipeline;

public:
	ComposeTAAPipeline() {
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/ComposeTAA.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/ComposeTAA.frag.spv"),
			.attachments = {Format::BGRA8Unorm}
		});
	}

	void Use(Buffer& viewBuffer, Image lastComposeTAA, Image currentLight, GBuffer& gbuffer) {
		PushConstant pc = {};
		pc.ViewBufferRID = GetRID(viewBuffer);
		pc.MotionTextureRID = GetRID(gbuffer.motion);
		pc.LastColorTextureRID = GetRID(lastComposeTAA);
		pc.ColorTextureRID = GetRID(currentLight);
		CmdPush(pc);

		CmdBind(pipeline);
		CmdDraw(6, 1, 0, 0);
	}

	static ComposeTAAPipeline& Get() {
		static ComposeTAAPipeline Instance;
		return Instance;
	}

};
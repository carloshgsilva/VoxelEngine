#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class LightBloomStepPipeline {
	Pipeline pipeline;

	struct PushConstant {
		int LightTextureRID;
	};

public:
	LightBloomStepPipeline() {
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightBloomStep.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightBloomStep.frag.spv"),
			.attachments = {Format::RGBA16Sfloat},
		});
	}

	void Use(Image& light) {
		CmdBind(pipeline);

		PushConstant pc;
		pc.LightTextureRID = GetRID(light);

		CmdPush(pc);
		CmdDraw(6, 1, 0, 0);
	}

	static LightBloomStepPipeline& Get() {
		static LightBloomStepPipeline Instance;
		return Instance;
	}
};
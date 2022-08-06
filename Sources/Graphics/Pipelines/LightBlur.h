#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class LightBlurPipeline {
	Pipeline pipeline;

	struct PushConstant {
		int LightTextureRID;
		int Horizontal;
	};

public:
	LightBlurPipeline() {
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightBlur.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightBlur.frag.spv"),
			.attachments = {Format::RGBA16Sfloat}
		});
	}

	void Use(Image& light, bool horizontal) {
		CmdBind(pipeline);

		PushConstant pc;
		pc.LightTextureRID = GetRID(light);
		pc.Horizontal = horizontal ? 1 : 0;

		CmdPush(pc);
		CmdDraw(6, 1, 0, 0);
	}

	static LightBlurPipeline& Get() {
		static LightBlurPipeline Instance;
		return Instance;
	}
};
#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class PresentPipeline {
	Pipeline pipeline;

	struct PushConstant {
		int ColorTextureRID;
	};

public:
	PresentPipeline() {
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Sources/Shaders/Bin/Present.vert.spv"),
			.FS = FileUtil::ReadBytes("Sources/Shaders/Bin/Present.frag.spv"),
			.attachments = {Format::BGRA8Unorm},
		});
	}

	void Use(Image& color) {
		CmdBind(pipeline);
		PushConstant pc;
		pc.ColorTextureRID = GetRID(color);

		CmdPush(pc);
		CmdDraw(6, 1, 0, 0);
	}

	static PresentPipeline& Get() {
		static PresentPipeline Instance;
		return Instance;
	}
};
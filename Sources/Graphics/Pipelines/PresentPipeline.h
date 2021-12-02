#pragma once
#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class PresentPipeline {
	GraphicsPipeline _Pipeline;

	struct PushConstant {
		int ColorTextureRID;
	};

public:
	PresentPipeline() {
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Present())
			.vertexShader(FileUtil::ReadBytes("Sources/Shaders/Bin/Present.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Sources/Shaders/Bin/Present.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Image& color) {
		cmd.bind(_Pipeline);
		PushConstant pc;
		pc.ColorTextureRID = color.getRID();

		cmd.constant(&pc, sizeof(PushConstant), 0);
		cmd.draw(6, 1, 0, 0);
	}

	static PresentPipeline& Get() {
		static PresentPipeline Instance;
		return Instance;
	}
};
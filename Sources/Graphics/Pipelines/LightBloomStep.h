#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class LightBloomStepPipeline {
	GraphicsPipeline _Pipeline;

	struct PushConstant {
		int LightTextureRID;
	};

public:
	LightBloomStepPipeline() {
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Light())
			.vertexShader(FileUtil::ReadBytes("Mods/default/Shaders/LightBloomStep.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Mods/default/Shaders/LightBloomStep.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Image& light) {
		cmd.bind(_Pipeline);

		PushConstant pc;
		pc.LightTextureRID = light.getRID();

		cmd.constant(&pc, sizeof(PushConstant), 0);
		cmd.draw(6, 1, 0, 0);
	}

	static LightBloomStepPipeline& Get() {
		static LightBloomStepPipeline Instance;
		return Instance;
	}
};
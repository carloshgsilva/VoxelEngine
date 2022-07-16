#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class LightBlurPipeline {
	GraphicsPipeline _Pipeline;

	struct PushConstant {
		int LightTextureRID;
		int Horizontal;
	};

public:
	LightBlurPipeline() {
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Light())
			.vertexShader(FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightBlur.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightBlur.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Image& light, bool horizontal) {
		cmd.bind(_Pipeline);

		PushConstant pc;
		pc.LightTextureRID = light.getRID();
		pc.Horizontal = horizontal ? 1 : 0;

		cmd.constant(&pc, sizeof(PushConstant), 0);
		cmd.draw(6, 1, 0, 0);
	}

	static LightBlurPipeline& Get() {
		static LightBlurPipeline Instance;
		return Instance;
	}
};
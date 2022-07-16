#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class ColorWorldPipeline {
	GraphicsPipeline _Pipeline;

	struct PushConstant {
		int ColorTextureRID;
		int OutlineTextureRID;
	};

public:
	ColorWorldPipeline() {
		_Pipeline = GraphicsPipeline::Create( GraphicsPipeline::Info()
			.setPass(Passes::Color())
			.vertexShader(FileUtil::ReadBytes("Assets/Mods/default/Shaders/ColorWorld.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Assets/Mods/default/Shaders/ColorWorld.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Image& compose, Framebuffer& outlineFB) {
		cmd.bind(_Pipeline);
		
		PushConstant pc;
		pc.ColorTextureRID = compose.getRID();
		pc.OutlineTextureRID = outlineFB.getAttachment(Passes::Outline_Color).getRID();
	
		cmd.constant(&pc, sizeof(PushConstant), 0);
		cmd.draw(6, 1, 0, 0);
	}

	static ColorWorldPipeline& Get() {
		static ColorWorldPipeline Instance;
		return Instance;
	}
};
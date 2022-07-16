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

	GraphicsPipeline _Pipeline;

public:
	ComposeTAAPipeline() {
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Color())
			.vertexShader(FileUtil::ReadBytes("Assets/Mods/default/Shaders/ComposeTAA.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Assets/Mods/default/Shaders/ComposeTAA.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Buffer& viewBuffer, Framebuffer& LastComposeTAAFB, Framebuffer& CurrentLightFB, Framebuffer& GeometryFB) {
		PushConstant pc = {};
		pc.ViewBufferRID = viewBuffer.getRID();
		pc.MotionTextureRID = GeometryFB.getAttachment(Passes::Geometry_Motion).getRID();
		pc.LastColorTextureRID = LastComposeTAAFB.getAttachment(0).getRID();
		pc.ColorTextureRID = CurrentLightFB.getAttachment(0).getRID();
		cmd.constant(&pc, sizeof(PushConstant), 0);

		cmd.bind(_Pipeline);
		cmd.draw(6, 1, 0, 0);
	}

	static ComposeTAAPipeline& Get() {
		static ComposeTAAPipeline Instance;
		return Instance;
	}

};
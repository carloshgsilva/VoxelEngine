#pragma once

#include "Graphics/Graphics.h"
#include "glm/glm.hpp"
#include "Graphics/Renderer/View.h"
#include "Core/Engine.h"
#include "Util/FileUtil.h"
#include <functional>

class ComposePipeline {
	GraphicsPipeline _Pipeline;

	struct PushConstant {
		int ViewBufferRID;
		int ColorTextureRID;
		int DepthTextureRID;
		int NormalTextureRID;
		int LightTextureRID;
		int ReflectionTextureRID;
		int Bloom0TextureRID;
		int Bloom1TextureRID;
		int Bloom2TextureRID;
		int Bloom3TextureRID;
		int Bloom4TextureRID;
	};

public:
	ComposePipeline() {
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Color())
			.vertexShader(FileUtil::ReadBytes("Mods/default/Shaders/Compose.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Mods/default/Shaders/Compose.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Buffer& viewBuffer, Framebuffer& geometryFB, Framebuffer& lightFB, Framebuffer& reflectionFB, std::vector<Framebuffer>& bloomFBs) {
		CHECK(bloomFBs.size() == 5);
		//Update Push Constances
		PushConstant pc = {};
		pc.ViewBufferRID = viewBuffer.getRID();
		pc.ColorTextureRID = geometryFB.getAttachment(Passes::Geometry_Color).getRID();
		pc.DepthTextureRID = geometryFB.getAttachment(Passes::Geometry_Depth).getRID();
		pc.NormalTextureRID = geometryFB.getAttachment(Passes::Geometry_Normal).getRID();
		pc.LightTextureRID = lightFB.getAttachment(0).getRID();
		pc.ReflectionTextureRID = reflectionFB.getAttachment(0).getRID();
		pc.Bloom0TextureRID = bloomFBs[0].getAttachment(0).getRID();
		pc.Bloom1TextureRID = bloomFBs[1].getAttachment(0).getRID();
		pc.Bloom2TextureRID = bloomFBs[2].getAttachment(0).getRID();
		pc.Bloom3TextureRID = bloomFBs[3].getAttachment(0).getRID();
		pc.Bloom4TextureRID = bloomFBs[4].getAttachment(0).getRID();

		cmd.constant((void*)&pc, sizeof(PushConstant), 0);


		cmd.bind(_Pipeline);

		constexpr bool ENABLE_UPSAMPLING = false;
		cmd.draw(6, 1, 0, 0);
	}

	static ComposePipeline& Get() {
		static ComposePipeline Instance;
		return Instance;
	}
};
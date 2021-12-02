#pragma once


#include "Graphics/Graphics.h"
#include "glm/glm.hpp"
#include "Util/FileUtil.h"
#include <functional>
#include "Core/Engine.h"

#include "Graphics/Renderer/View.h"

class LightReflectionPipeline {
	struct PushConstant {
		int ViewBufferRID;
		int LightTextureRID;
		int NormalTextureRID;
		int MaterialTextureRID;
		int DepthTextureRID;
		int BlueNoiseTextureRID;
		int SkyBoxTextureRID;
		int ShadowVoxRID;
	};

	GraphicsPipeline _Pipeline;
public:
	LightReflectionPipeline() {
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Light())
			.vertexShader(FileUtil::ReadBytes("Mods/default/Shaders/LightReflection.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Mods/default/Shaders/LightReflection.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Buffer& viewBuffer, Image& lightDiffuse, Framebuffer& geometryFB, Image& shadowVox, Image& skybox, Image& blueNoise) {
		//Update Push Constances
		PushConstant pc = {};
		pc.ViewBufferRID = viewBuffer.getRID();
		pc.LightTextureRID = lightDiffuse.getRID();
		pc.NormalTextureRID = geometryFB.getAttachment(Passes::Geometry_Normal).getRID();
		pc.MaterialTextureRID = geometryFB.getAttachment(Passes::Geometry_Material).getRID();
		pc.DepthTextureRID = geometryFB.getAttachment(Passes::Geometry_Depth).getRID();
		pc.BlueNoiseTextureRID = blueNoise.getRID();
		pc.ShadowVoxRID = shadowVox.getRID();
		pc.SkyBoxTextureRID = skybox.getRID();

		//TODO: Fill with
		cmd.constant(&pc, sizeof(PushConstant), 0);

		cmd.bind(_Pipeline);
		cmd.draw(6, 1, 0, 0);
	}

	static LightReflectionPipeline& Get() {
		static LightReflectionPipeline Instance;
		return Instance;
	}

};
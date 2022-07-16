#pragma once


#include "Graphics/Graphics.h"
#include "glm/glm.hpp"
#include "Util/FileUtil.h"
#include <functional>
#include "Core/Engine.h"

#include "Graphics/Renderer/View.h"

class LightAmbientPipeline {
	struct PushConstant {
		int ViewBufferRID;
		int ColorTextureRID;
		int NormalTextureRID;
		int MaterialTextureRID;
		int DepthTextureRID;
		int BlueNoiseTextureRID;
		int ShadowVoxRID;
		int SkyBoxRID;
	};

	GraphicsPipeline _Pipeline;
public:
	LightAmbientPipeline() {
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Light())
			.setBlend(Blend::Additive)
			.vertexShader(FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightAmbient.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightAmbient.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Buffer& viewBuffer, Framebuffer& geometryFB, Image& shadowVox, Image& blueNoise, Image& skyBox) {
		//Update Push Constances
		PushConstant pc = {};
		pc.ViewBufferRID = viewBuffer.getRID();
		pc.ColorTextureRID = geometryFB.getAttachment(Passes::Geometry_Color).getRID();
		pc.NormalTextureRID = geometryFB.getAttachment(Passes::Geometry_Normal).getRID();
		pc.MaterialTextureRID = geometryFB.getAttachment(Passes::Geometry_Material).getRID();
		pc.DepthTextureRID = geometryFB.getAttachment(Passes::Geometry_Depth).getRID();
		pc.BlueNoiseTextureRID = blueNoise.getRID();
		pc.ShadowVoxRID = shadowVox.getRID();
		pc.SkyBoxRID = skyBox.getRID();

		//TODO: Fill with
		cmd.constant(&pc, sizeof(PushConstant), 0);

		cmd.bind(_Pipeline);
		cmd.draw(6, 1, 0, 0);
	}

	static LightAmbientPipeline& Get() {
		static LightAmbientPipeline Instance;
		return Instance;
	}

};
#pragma once


#include "Graphics/Graphics.h"
#include "glm/glm.hpp"
#include <functional>

#include "Graphics/Renderer/View.h"

class LightTAAPipeline {
	GraphicsPipeline _Pipeline;

	struct PushConstant {
		int ViewBufferRID;
		int BlueNoiseTextureRID;
		int MotionTextureRID;
		int LightTextureRID;
		int ColorTextureRID;
		int NormalTextureRID;
		int MaterialTextureRID;
		int DepthTextureRID;
		int LastLightTextureRID;
	};

public:
	LightTAAPipeline() {
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Light())
			.vertexShader(FileUtil::ReadBytes("Mods/default/Shaders/LightTAA.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Mods/default/Shaders/LightTAA.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Buffer& viewbuffer, Image& blueNoise, Framebuffer& LastLightTAAFB, Framebuffer& LightFB, Framebuffer& GeometryFB) {
		
		PushConstant pc;
		pc.ViewBufferRID = viewbuffer.getRID();
		pc.BlueNoiseTextureRID = blueNoise.getRID();
		pc.MotionTextureRID = GeometryFB.getAttachment(Passes::Geometry_Motion).getRID();
		pc.LightTextureRID = LightFB.getAttachment(0).getRID();
		pc.ColorTextureRID = GeometryFB.getAttachment(Passes::Geometry_Color).getRID();
		pc.NormalTextureRID = GeometryFB.getAttachment(Passes::Geometry_Normal).getRID();
		pc.MaterialTextureRID = GeometryFB.getAttachment(Passes::Geometry_Material).getRID();
		pc.DepthTextureRID = GeometryFB.getAttachment(Passes::Geometry_Depth).getRID();
		pc.LastLightTextureRID = LastLightTAAFB.getAttachment(0).getRID();
		
		cmd.bind(_Pipeline);
		cmd.constant(&pc, sizeof(PushConstant), 0);
		cmd.draw(6, 1, 0, 0);
		
	}

	static LightTAAPipeline& Get() {
		static LightTAAPipeline Instance;
		return Instance;
	}

};
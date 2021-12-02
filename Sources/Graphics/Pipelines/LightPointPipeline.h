#pragma once

#pragma once


#include "Graphics/Graphics.h"
#include "Graphics/Renderer/View.h"
#include "Util/FileUtil.h"
#include "glm/glm.hpp"
#include <functional>


//TODO: LightPointPipeline
class LightPointPipeline {
	constexpr static inline int MAX_POINT_LIGHTS = 64;
	GraphicsPipeline _Pipeline;
	Buffer _DataBuffer;

	//TODO: Store in the PushConstant
	struct PointLight {
		glm::vec3 Position;
		float Range;
		glm::vec3 Color;
		float Attenuation;
	};

	struct Data {
		PointLight Lights[MAX_POINT_LIGHTS];
	};
	int _CurrentLightIndex = 0;
	Data _Data;

	struct PushConstant {
		int ViewBufferRID;
		int ColorTextureRID;
		int NormalTextureRID;
		int MaterialTextureRID;
		int DepthTextureRID;
		int BlueNoiseTextureRID;
		int ShadowVoxRID;
		int PointLightsBufferRID;
		int LightIndex;
	};


public:

	LightPointPipeline() {
		_DataBuffer = Buffer::Create(sizeof(Data), BufferUsage::Storage, MemoryType::CPU_TO_GPU);
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Light())
			.setBlend(Blend::Additive)
			.vertexShader(FileUtil::ReadBytes("Mods/default/Shaders/LightPoint.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Mods/default/Shaders/LightPoint.frag.spv"))
		);
	}
	
	void DrawLight(glm::vec3 position, float range, glm::vec3 color, float attenuation) {

		if (_CurrentLightIndex >= MAX_POINT_LIGHTS) {
			Log::warn("Max number of Point Lights reached! Max is {}", MAX_POINT_LIGHTS);
		}
		else {
			PointLight& sl = _Data.Lights[_CurrentLightIndex];
			sl.Position = position;
			sl.Range = range;
			sl.Color = color;
			sl.Attenuation = attenuation;

			_CurrentLightIndex++;
		}
	}

	void Use(CmdBuffer& cmd, Buffer& viewBuffer, Framebuffer& geometryFB, Image& ShadowVox, Image& BlueNoise, int Frame, std::function<void(LightPointPipeline& P)> cb) {
		_CurrentLightIndex = 0;

		PushConstant pc;
		pc.ViewBufferRID = viewBuffer.getRID();
		pc.ColorTextureRID = geometryFB.getAttachment(Passes::Geometry_Color).getRID();
		pc.NormalTextureRID = geometryFB.getAttachment(Passes::Geometry_Normal).getRID();
		pc.MaterialTextureRID = geometryFB.getAttachment(Passes::Geometry_Material).getRID();
		pc.DepthTextureRID = geometryFB.getAttachment(Passes::Geometry_Depth).getRID();
		pc.BlueNoiseTextureRID = BlueNoise.getRID();
		pc.ShadowVoxRID = ShadowVox.getRID();
		pc.PointLightsBufferRID = _DataBuffer.getRID();

		cb(*this);

		//Send to GPU
		_DataBuffer.update(&_Data, sizeof(Data));

		//Fill cmds
		cmd.bind(_Pipeline);

		for (int i = 0; i < _CurrentLightIndex; i++) {
			pc.LightIndex = i;
			cmd.constant((void*)&pc, sizeof(PushConstant), 0);
			cmd.draw(6, 1, 0, 0);
		}
	}

	static LightPointPipeline& Get() {
		static LightPointPipeline Instance;
		return Instance;
	}
	

};
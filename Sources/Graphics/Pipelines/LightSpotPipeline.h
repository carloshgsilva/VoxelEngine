#pragma once

#pragma once


#include "Graphics/Graphics.h"
#include "Graphics/Renderer/View.h"
#include "glm/glm.hpp"
#include <functional>


//TODO: LightPointPipeline
class LightSpotPipeline {
	constexpr static inline int MAX_SPOT_LIGHTS = 64;
	GraphicsPipeline _Pipeline;
	Buffer _DataBuffer;

	//TODO: Store in the PushConstant
	struct SpotLight {
		glm::vec3 Position;
		float Range;
		glm::vec3 Color;
		float Attenuation;
		glm::vec3 Direction;
		float Angle;
		float AngleAttenuation;
		float PADDING_0[3];
	};

	struct Data {
		SpotLight Lights[MAX_SPOT_LIGHTS];
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
		int SpotLightsBufferRID;
		int LightIndex;
	};

public:

	LightSpotPipeline() {
		_DataBuffer = Buffer::Create(sizeof(Data), BufferUsage::Storage, MemoryType::CPU_TO_GPU);
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Light())
			.setBlend(Blend::Additive)
			.vertexShader(FileUtil::ReadBytes("Mods/default/Shaders/LightSpot.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Mods/default/Shaders/LightSpot.frag.spv"))
		);
	}
	
	void DrawLight(glm::vec3 position, float range, glm::vec3 color, float attenuation, glm::vec3 dir, float angle, float angleAttenuation) {

		if (_CurrentLightIndex >= MAX_SPOT_LIGHTS) {
			Log::warn("Max number of Spot Lights reached! Max is {}", MAX_SPOT_LIGHTS);
		}
		else {
			SpotLight& sl = _Data.Lights[_CurrentLightIndex];
			sl.Position = position;
			sl.Range = range;
			sl.Color = color;
			sl.Attenuation = attenuation;
			sl.Direction = dir;
			sl.Angle = angle;
			sl.AngleAttenuation = angleAttenuation;

			_CurrentLightIndex++;
		}
	}

	void Use(CmdBuffer& cmd, Buffer& viewBuffer, Framebuffer& geometryFB, Image& ShadowVox, Image& BlueNoise, int Frame, std::function<void(LightSpotPipeline& P)> cb) {
		_CurrentLightIndex = 0;

		PushConstant pc;
		pc.ViewBufferRID = viewBuffer.getRID();
		pc.ColorTextureRID = geometryFB.getAttachment(Passes::Geometry_Color).getRID();
		pc.NormalTextureRID = geometryFB.getAttachment(Passes::Geometry_Normal).getRID();
		pc.MaterialTextureRID = geometryFB.getAttachment(Passes::Geometry_Material).getRID();
		pc.DepthTextureRID = geometryFB.getAttachment(Passes::Geometry_Depth).getRID();
		pc.BlueNoiseTextureRID = BlueNoise.getRID();
		pc.ShadowVoxRID = ShadowVox.getRID();
		pc.SpotLightsBufferRID = _DataBuffer.getRID();

		cb(*this);

		//Send to GPU
		_DataBuffer.update(&_Data, sizeof(Data));

		//Fill cmds
		cmd.bind(_Pipeline);
		for (int i = 0; i < _CurrentLightIndex; i++) {
			pc.LightIndex = i;
			cmd.constant(&pc, sizeof(PushConstant), 0);
			cmd.draw(6, 1, 0, 0);
		}
	}

	static LightSpotPipeline& Get() {
		static LightSpotPipeline Instance;
		return Instance;
	}
	

};
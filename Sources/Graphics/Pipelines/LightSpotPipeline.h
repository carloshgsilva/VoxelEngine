#pragma once

#pragma once


#include "Graphics/Graphics.h"
#include "Graphics/Renderer/View.h"
#include "glm/glm.hpp"
#include <functional>


//TODO: LightPointPipeline
class LightSpotPipeline {
	constexpr static inline int MAX_SPOT_LIGHTS = 64;
	Pipeline pipeline;
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
		_DataBuffer = CreateBuffer({
			.size = sizeof(Data),
			.usage = BufferUsage::Storage,
			.memoryType = MemoryType::CPU_TO_GPU
		});
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightSpot.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightSpot.frag.spv"),
			.attachments = {Format::RGBA16Sfloat},
			.blends = {Blend::Additive}
		});
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

	void Use(Buffer& viewBuffer, GBuffer& gbuffer, Image& ShadowVox, Image& BlueNoise, int Frame, std::function<void(LightSpotPipeline& P)> cb) {
		_CurrentLightIndex = 0;

		PushConstant pc;
		pc.ViewBufferRID = GetRID(viewBuffer);
		pc.ColorTextureRID = GetRID(gbuffer.color);
		pc.NormalTextureRID = GetRID(gbuffer.normal);
		pc.MaterialTextureRID = GetRID(gbuffer.material);
		pc.DepthTextureRID = GetRID(gbuffer.depth);
		pc.BlueNoiseTextureRID = GetRID(BlueNoise);
		pc.ShadowVoxRID = GetRID(ShadowVox);
		pc.SpotLightsBufferRID = GetRID(_DataBuffer);

		cb(*this);

		//Send to GPU
		WriteBuffer(_DataBuffer, &_Data, sizeof(Data));

		//Fill cmds
		CmdBind(pipeline);
		for (int i = 0; i < _CurrentLightIndex; i++) {
			pc.LightIndex = i;
			CmdPush(pc);
			CmdDraw(6, 1, 0, 0);
		}
	}

	static LightSpotPipeline& Get() {
		static LightSpotPipeline Instance;
		return Instance;
	}
	

};
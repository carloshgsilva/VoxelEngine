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
	Pipeline pipeline;
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
		_DataBuffer = CreateBuffer({
			.size = sizeof(Data),
			.usage = BufferUsage::Storage,
			.memoryType = MemoryType::CPU_TO_GPU
		});
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightPoint.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightPoint.frag.spv"),
			.attachments = {Format::RGBA16Sfloat},
			.blends = {Blend::Additive}
		});
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

	void Use(Buffer& viewBuffer, GBuffer& gbuffer, Image& ShadowVox, Image& BlueNoise, int Frame, std::function<void(LightPointPipeline& P)> cb) {
		_CurrentLightIndex = 0;

		PushConstant pc;
		pc.ViewBufferRID = GetRID(viewBuffer);
		pc.ColorTextureRID = GetRID(gbuffer.color);
		pc.NormalTextureRID = GetRID(gbuffer.normal);
		pc.MaterialTextureRID = GetRID(gbuffer.material);
		pc.DepthTextureRID = GetRID(gbuffer.depth);
		pc.BlueNoiseTextureRID = GetRID(BlueNoise);
		pc.ShadowVoxRID = GetRID(ShadowVox);
		pc.PointLightsBufferRID = GetRID(_DataBuffer);

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

	static LightPointPipeline& Get() {
		static LightPointPipeline Instance;
		return Instance;
	}
	

};
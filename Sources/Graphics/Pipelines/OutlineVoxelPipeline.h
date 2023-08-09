#pragma once

#include "Graphics/Graphics.h"
#include "Graphics/Renderer/View.h"

#include "Core/Window.h"
#include "Core/Engine.h"

#include "World/Components.h"
#include "Profiler/Profiler.h"

#include <glm/glm.hpp>

class OutlineVoxelPipeline {
	Pipeline pipeline;

	struct PushConstant {
		int ViewBufferRID;
		int VolumeRID;
		int GBufferRID;
		int PADDING_0;
		glm::mat4 WorldMatrix;
		glm::vec3 Color;
	};

public:

	struct Cmd {
		glm::mat4 WorldMatrix;
		glm::vec3 Color;
		int VolumeRID;
	};

	OutlineVoxelPipeline() {
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/OutlineVoxel.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/OutlineVoxel.frag.spv"),
			.attachments = {Format::BGRA8Unorm}
		});
	}

	void Use(Buffer& viewBuffer, GBuffer& gbuffer, std::vector<Cmd>& instances) {

		CmdBind(pipeline);
		PushConstant pc{};
		pc.ViewBufferRID = GetRID(viewBuffer);
		pc.GBufferRID = GetRID(gbuffer.packed);

		//Upload the buffered draw data
		for (auto& c : instances) {
			pc.VolumeRID = c.VolumeRID;
			pc.WorldMatrix = c.WorldMatrix;
			pc.Color = c.Color;

			CmdPush(pc);
			CmdDraw(36, 1, 0, 0);
		}
	}

	static OutlineVoxelPipeline& Get() {
		static OutlineVoxelPipeline Instance;
		return Instance;
	}

};
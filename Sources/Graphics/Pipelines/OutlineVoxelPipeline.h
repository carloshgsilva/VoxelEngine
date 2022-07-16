#pragma once

#include "Graphics/Graphics.h"
#include "Graphics/Renderer/View.h"

#include "Core/Window.h"
#include "Core/Engine.h"

#include "World/Components.h"
#include "Profiler/Profiler.h"

#include <glm/glm.hpp>

class OutlineVoxelPipeline {
	GraphicsPipeline _Pipeline;

	struct PushConstant {
		int ViewBufferRID;
		int VolumeRID;
		int DepthTextureRID;
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
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Outline())
			.vertexShader(FileUtil::ReadBytes("Assets/Mods/default/Shaders/OutlineVoxel.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Assets/Mods/default/Shaders/OutlineVoxel.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Buffer& viewBuffer, Framebuffer& geometryFB, std::vector<Cmd>& instances) {

		cmd.bind(_Pipeline);
		PushConstant pc{};
		pc.ViewBufferRID = viewBuffer.getRID();
		pc.DepthTextureRID = geometryFB.getAttachment(Passes::Geometry_Depth).getRID();

		{
			PROFILE_SCOPE("Cmd Build");
			//Upload the buffered draw data
			for (auto& c : instances) {
				pc.VolumeRID = c.VolumeRID;
				pc.WorldMatrix = c.WorldMatrix;
				pc.Color = c.Color;

				cmd.constant(&pc, sizeof(PushConstant), 0);
				cmd.draw(36, 1, 0, 0);
			}
		}
	}

	static OutlineVoxelPipeline& Get() {
		static OutlineVoxelPipeline Instance;
		return Instance;
	}

};
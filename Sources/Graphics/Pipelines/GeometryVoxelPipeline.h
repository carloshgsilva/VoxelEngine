#pragma once

#include "Graphics/Graphics.h"
#include "Graphics/Renderer/View.h"

#include "Core/Window.h"
#include "Core/Engine.h"

#include "World/Components.h"
#include "Profiler/Profiler.h"
#include "Util/FileUtil.h"

#include <glm/glm.hpp>

class GeometryVoxelPipeline {
	GraphicsPipeline _Pipeline;
	constexpr static inline int MAX_INSTANCES = 4096;
	Buffer _VoxCmdsBuffer;

	struct PushConstant {
		int ViewBufferRID;
		int VoxCmdsBufferRID;
		int BlueNoiseRID;
		int VoxCmdIndex;
	};

public:

	//Buffers are padded by 16 bytes eg vec4, mat4, float[4], int[4]
	struct Cmd {
		glm::mat4 WorldMatrix;
		glm::mat4 LastWorldMatrix;
		int VolumeRID;
		int PalleteIndex;
		int PADDING[2];
	};

	GeometryVoxelPipeline() {
		_VoxCmdsBuffer = Buffer::Create(sizeof(Cmd)*MAX_INSTANCES, BufferUsage::Storage, MemoryType::CPU_TO_GPU);
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Geometry())
			.setDepth()
			.vertexShader(FileUtil::ReadBytes("Mods/default/Shaders/GeometryVoxel.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Mods/default/Shaders/GeometryVoxel.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Buffer& viewBuffer, glm::vec2 res, Image& blueNoise, std::vector<Cmd>& cmds) {

		//Send to GPU
		_VoxCmdsBuffer.update(cmds.data(), sizeof(Cmd)*cmds.size());

		PushConstant pc;
		pc.ViewBufferRID = viewBuffer.getRID();
		pc.VoxCmdsBufferRID = _VoxCmdsBuffer.getRID();
		pc.BlueNoiseRID = blueNoise.getRID();

		cmd.bind(_Pipeline);
		{
			PROFILE_SCOPE("Cmd Build");
			//Upload the buffered draw data
			int id = 0;
			for (Cmd& c : cmds) {
				pc.VoxCmdIndex = id++;
				cmd.constant(&pc, sizeof(PushConstant), 0);
				cmd.draw(36, 1, 0, 0);
			}
		}
		
	}

	static GeometryVoxelPipeline& Get() {
		static GeometryVoxelPipeline Instance;
		return Instance;
	}

};
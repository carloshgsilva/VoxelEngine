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
	Pipeline pipeline;
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
		_VoxCmdsBuffer = CreateBuffer({
			.size = sizeof(Cmd)*MAX_INSTANCES,
			.usage = BufferUsage::Storage,
			.memoryType = MemoryType::CPU_TO_GPU
		});
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/GeometryVoxel.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/GeometryVoxel.frag.spv"),
			.attachments = GBuffer::Attachments(),
			.depthTest = true,
			.depthWrite = true
		});
	}

	void Use(Buffer& viewBuffer, glm::vec2 res, Image& blueNoise, std::vector<Cmd>& cmds) {

		//Send to GPU
		WriteBuffer(_VoxCmdsBuffer, cmds.data(), sizeof(Cmd)*cmds.size());

		PushConstant pc;
		pc.ViewBufferRID = GetRID(viewBuffer);
		pc.VoxCmdsBufferRID = GetRID(_VoxCmdsBuffer);
		pc.BlueNoiseRID = GetRID(blueNoise);

		CmdBind(pipeline);
		{
			PROFILE_SCOPE("Cmd Build");
			//Upload the buffered draw data
			int id = 0;
			for (Cmd& c : cmds) {
				pc.VoxCmdIndex = id++;
				CmdPush(pc);
				CmdDraw(36, 1, 0, 0);
			}
		}
		
	}

	static GeometryVoxelPipeline& Get() {
		static GeometryVoxelPipeline Instance;
		return Instance;
	}

};
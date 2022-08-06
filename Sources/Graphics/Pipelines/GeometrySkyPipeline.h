#pragma once

#include "Graphics/Graphics.h"
#include "Graphics/Renderer/View.h"

#include "Core/Window.h"
#include "Core/Engine.h"

#include "World/Components.h"
#include "Profiler/Profiler.h"
#include "Util/FileUtil.h"

#include <glm/glm.hpp>

class GeometrySkyPipeline {
	Pipeline pipeline;

	struct PushConstant {
		int ViewBufferRID;
	};

public:
	GeometrySkyPipeline() {
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/GeometrySky.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/GeometrySky.frag.spv"),
			.attachments = GBuffer::Attachments(),
			.depthTest = true,
			.depthWrite = true
		});
	}

	void Use(Buffer& viewBuffer, glm::vec2 res) {
		CmdBind(pipeline);

		PushConstant pc;
		pc.ViewBufferRID = GetRID(viewBuffer);

		CmdPush(pc);
		CmdDraw(6, 1, 0, 0);
	}

	static GeometrySkyPipeline& Get() {
		static GeometrySkyPipeline Instance;
		return Instance;
	}

};
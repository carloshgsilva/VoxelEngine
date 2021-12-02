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
	GraphicsPipeline _Pipeline;

	struct PushConstant {
		int ViewBufferRID;
	};

public:
	GeometrySkyPipeline() {
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::Geometry())
			.vertexShader(FileUtil::ReadBytes("Mods/default/Shaders/GeometrySky.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Mods/default/Shaders/GeometrySky.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Buffer& viewBuffer, glm::vec2 res) {
		cmd.bind(_Pipeline);

		PushConstant pc;
		pc.ViewBufferRID = viewBuffer.getRID();

		cmd.constant(&pc, sizeof(PushConstant), 0);
		cmd.draw(6, 1, 0, 0);
	}

	static GeometrySkyPipeline& Get() {
		static GeometrySkyPipeline Instance;
		return Instance;
	}

};
#pragma once

#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class CubeMapFacePipeline {
	GraphicsPipeline _Pipeline;

	struct PushConstant {
		int PanoramaTextureRID;
		int FaceID;
	};

public:
	CubeMapFacePipeline() {
		_Pipeline = GraphicsPipeline::Create(GraphicsPipeline::Info()
			.setPass(Passes::CubeMapFace())
			.vertexShader(FileUtil::ReadBytes("Mods/default/Shaders/CubeMapFace.vert.spv"))
			.fragmentShader(FileUtil::ReadBytes("Mods/default/Shaders/CubeMapFace.frag.spv"))
		);
	}

	void Use(CmdBuffer& cmd, Image& panorama, int face) {
		cmd.bind(_Pipeline);

		PushConstant pc;
		pc.PanoramaTextureRID = panorama.getRID();
		pc.FaceID = face;

		cmd.constant(&pc, sizeof(PushConstant), 0);
		cmd.draw(6, 1, 0, 0);
	}

	static CubeMapFacePipeline& Get() {
		static CubeMapFacePipeline Instance;
		return Instance;
	}
};
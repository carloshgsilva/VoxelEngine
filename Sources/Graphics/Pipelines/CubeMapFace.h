#pragma once

#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class CubeMapFacePipeline {
	Pipeline pipeline;

	struct PushConstant {
		int PanoramaTextureRID;
		int FaceID;
	};

public:
	CubeMapFacePipeline() {
		pipeline = CreatePipeline({
			.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/CubeMapFace.vert.spv"),
			.FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/CubeMapFace.frag.spv"),
			.attachments = {Format::RGBA16Sfloat}
		});
	}

	void Use(Image& panorama, int face) {
		CmdBind(pipeline);

		PushConstant pc;
		pc.PanoramaTextureRID = GetRID(panorama);
		pc.FaceID = face;

		CmdPush(&pc, sizeof(PushConstant), 0);
		CmdDraw(6, 1, 0, 0);
	}

	static CubeMapFacePipeline& Get() {
		static CubeMapFacePipeline Instance;
		return Instance;
	}
};
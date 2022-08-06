#pragma once

#include <inttypes.h>
#include <tuple>

#include "Core/Core.h"
#include "Core/Module.h"

#include <evk/evk.h>

using namespace evk;

struct GBuffer {
	Image color;
	Image normal;
	Image material;
	Image motion;
	Image depth;

	GBuffer() {}
	GBuffer(uint32 width, uint32 height) {
		ImageDesc desc = {
			.extent = {width, height},
			.usage = ImageUsage::Sampled | ImageUsage::Attachment
		};

		desc.format = Format::BGRA8Unorm;
		color = CreateImage(desc);

		desc.format = Format::RGBA8Snorm;
		normal = CreateImage(desc);

		desc.format = Format::BGRA8Unorm;
		material = CreateImage(desc);
		
		desc.format = Format::RG16Sfloat;
		motion = CreateImage(desc);

		desc.format = Format::D24UnormS8Uint;
		depth = CreateImage(desc);
	}
	
	inline static std::vector<Format> Attachments() {
		return std::vector<Format>{
			Format::BGRA8Unorm,
			Format::RGBA8Snorm,
			Format::BGRA8Unorm,
			Format::RG16Sfloat,
			Format::D24UnormS8Uint
		};
	}

	template<typename T> void Render(T callback) {
		CmdBarrier(color, ImageLayout::Undefined, ImageLayout::Attachment);
		CmdBarrier(normal, ImageLayout::Undefined, ImageLayout::Attachment);
		CmdBarrier(material, ImageLayout::Undefined, ImageLayout::Attachment);
		CmdBarrier(motion, ImageLayout::Undefined, ImageLayout::Attachment);
		CmdBarrier(depth, ImageLayout::Undefined, ImageLayout::Attachment);
		CmdRender(
			{color, normal, material, motion, depth},
			{ClearColor{}, ClearColor{}, ClearColor{}, ClearColor{}, ClearDepthStencil{}},
			callback
		);
		CmdBarrier(color, ImageLayout::Attachment, ImageLayout::ShaderRead);
		CmdBarrier(normal, ImageLayout::Attachment, ImageLayout::ShaderRead);
		CmdBarrier(material, ImageLayout::Attachment, ImageLayout::ShaderRead);
		CmdBarrier(motion, ImageLayout::Attachment, ImageLayout::ShaderRead);
		CmdBarrier(depth, ImageLayout::Attachment, ImageLayout::ShaderRead);
	}
};

class Graphics : public ModuleDef<Graphics> {
public:
	Graphics();

	template<typename T>
	static void Frame(T callback) {
		
		evk::Submit();
	}
};
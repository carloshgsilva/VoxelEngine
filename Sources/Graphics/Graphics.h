#pragma once

#include <inttypes.h>
#include <tuple>

#include "Core/Core.h"
#include "Core/Module.h"

#include <evk/evk.h>

using namespace evk;

class Graphics : public ModuleDef<Graphics> {
	
	CmdBuffer frameCmdBuffer;
	CmdBuffer transferCmdBuffer;

public:
	Graphics();

	static const std::vector<TimestampEntry> GetTimestamps() { return Get().frameCmdBuffer.timestamps(); }

	template<typename T>
	static void Frame(T callback) {
		Get().frameCmdBuffer.use([&] { callback(Get().frameCmdBuffer); }).submit().wait();
	}

	//Used to synchonous transfer data from CPU to GPU
	template<typename T>
	static void Transfer(T callback) {
		Get().transferCmdBuffer.use([&] { callback(Get().transferCmdBuffer); }).submit().wait();
	}

	//TODO: Async transfer
};

class Passes {
public:
	static Pass& Present() {
		static Pass p = Pass::Create({
			Pass::Attachment(Format::B8G8R8A8Unorm).setPresent(true)
		});
		return p;
	}

	static inline constexpr int Geometry_Color = 0;
	static inline constexpr int Geometry_Normal = 1;
	static inline constexpr int Geometry_Material = 2;
	static inline constexpr int Geometry_Motion = 3;
	static inline constexpr int Geometry_Depth = 4;
	static Pass& Geometry() {
		static Pass p = Pass::Create({
			/* [0] Color     */ Pass::Attachment(Format::B8G8R8A8Unorm),
			/* [1] Normal    */ Pass::Attachment(Format::R8G8B8A8Snorm),
			/* [2] Material  */ Pass::Attachment(Format::B8G8R8A8Unorm),
			/* [3] Motion    */ Pass::Attachment(Format::R16G16Sfloat),
			/* [4] Depth     */ Pass::Attachment(Format::D24UnormS8Uint),
		});
		return p;
	}

	static inline constexpr int Outline_Color = 0;
	static inline constexpr int Outline_Depth = 1;
	static Pass& Outline() {
		static Pass p = Pass::Create({
			/* [0] Color     */ Pass::Attachment(Format::B8G8R8A8Unorm),
			/* [1] Depth     */ Pass::Attachment(Format::D24UnormS8Uint),
			});
		return p;
	}

	static Pass& Light() {
		static Pass p = Pass::Create({
			/* [0] */ Pass::Attachment(Format::R16G16B16A16Sfloat),
			});
		return p;
	}

	static Pass& Color() {
		static Pass p = Pass::Create({
			/* [0] */ Pass::Attachment(Format::B8G8R8A8Unorm),
			});
		return p;
	}

	static Pass& CubeMapFace() {
		static Pass p = Pass::Create({
			/* [0] */ Pass::Attachment(Format::R16G16B16A16Sfloat),
			});
		return p;
	}
};
#include "WorldRenderer.h"

#include "World/World.h"
#include "World/Components.h"
#include "World/Systems/CameraSystem.h"
#include "World/Systems/ShadowVoxSystem.h"

#include "Core/Engine.h"
#include "Core/Input.h"

#include "Graphics/Graphics.h"
#include "Profiler/Profiler.h"

#include "GLFW/glfw3.h"

#include "Editor/Importer/AssetImporter.h"

#include "Graphics/Pipelines/GeometrySkyPipeline.h"
#include "Graphics/Pipelines/GeometryVoxelPipeline.h"
#include "Graphics/Pipelines/LightAmbientPipeline.h"
#include "Graphics/Pipelines/LightPointPipeline.h"
#include "Graphics/Pipelines/LightSpotPipeline.h"
#include "Graphics/Pipelines/LightTAAPipeline.h"
#include "Graphics/Pipelines/LightReflectionPipeline.h"
#include "Graphics/Pipelines/LightBloomStep.h"
#include "Graphics/Pipelines/LightBlur.h"
#include "Graphics/Pipelines/OutlineVoxelPipeline.h"
#include "Graphics/Pipelines/ComposePipeline.h"
#include "Graphics/Pipelines/ComposeTAAPipeline.h"
#include "Graphics/Pipelines/ColorWorldPipeline.h"

constexpr bool ENABLE_UPSAMPLING = false;

const int BLOOM_MIP_COUNT = 5;

//TimeStamp Points

const int TS_GBUFFER = 0;
const int TS_OUTLINE = 1;
const int TS_LIGHTS = 2;
const int TS_LIGHTS_TAA = 3;
const int TS_REFLECTION = 4;
const int TS_COMPOSE = 5;
const int TS_COMPOSE_TAA = 6;
const int TS_TOTAL = 7;


struct RenderCmds {
	std::vector<OutlineVoxelPipeline::Cmd> outline;
	std::vector<GeometryVoxelPipeline::Cmd> voxel;

	void Clear() {
		outline.clear();
		voxel.clear();
	}
};

WorldRenderer::WorldRenderer() {
	//Create Initial Framebuffers
	RecreateFramebuffer(320, 240); //Small Initial buffer allocation

	Cmds = new RenderCmds();
	_BlueNoise = Assets::Load("default/LDR_RGBA_0.png");
	_DefaultSkyBox = Assets::Load("default/immenstadter_horn_4k.hdr");
	_ViewBuffer = Buffer::Create(sizeof(ViewData), BufferUsage::Storage, MemoryType::CPU_TO_GPU);
}

void WorldRenderer::CmdOutline(const glm::mat4& matrix, Image& vox, glm::vec3 color) {
	OutlineVoxelPipeline::Cmd c;
	c.WorldMatrix = matrix;
	c.VolumeRID = vox.getRID();
	c.Color = color;
	Cmds->outline.push_back(c);
}
void WorldRenderer::CmdVoxel(const glm::mat4& matrix, const glm::mat4& lastMatrix, Image& vox, int palleteIndex, int id) {
	GeometryVoxelPipeline::Cmd c;
	c.WorldMatrix = matrix;
	c.LastWorldMatrix = lastMatrix;
	c.VolumeRID = vox.getRID();
	c.PalleteIndex = palleteIndex;
	Cmds->voxel.push_back(c);
}


void WorldRenderer::RecreateFramebuffer(int32 Width, int32 Height) {

	if (Width <= 1)Width = 1;
	if (Height <= 1)Height = 1;

	if (ENABLE_UPSAMPLING) {
		Width /= 2;
		Height /= 2;
	}

	_Bloom1Buffer.clear();
	_Bloom2Buffer.clear();

	//Geometry
	_GeometryBuffer = Framebuffer::Create(Passes::Geometry(), Width, Height);

	//Outline
	_OutlineBuffer = Framebuffer::Create(Passes::Outline(), Width, Height);

	//Light
	_LastLightBuffer = Framebuffer::Create(Passes::Light(), Width, Height);
	_CurrentLightBuffer = Framebuffer::Create(Passes::Light(), Width, Height);
	_TAALightBuffer = Framebuffer::Create(Passes::Light(), Width, Height);
	_ReflectionBuffer = Framebuffer::Create(Passes::Light(), Width, Height);

	_BloomStepBuffer = Framebuffer::Create(Passes::Light(), Width >> 1, Height >> 1); // second mip
	for (int i = 0; i < BLOOM_MIP_COUNT; i++) {
		int mip = 2 + i; // third mip
		int w = std::max(Width >> mip, 1);
		int h = std::max(Height >> mip, 1);
		_Bloom1Buffer.push_back(Framebuffer::Create(Passes::Light(), w, h));
		_Bloom2Buffer.push_back(Framebuffer::Create(Passes::Light(), w, h));
	}

	if (ENABLE_UPSAMPLING) {
		Width *= 2;
		Height *= 2;
	}

	//Compose
	_LastComposeBuffer = Framebuffer::Create(Passes::Color(), Width, Height);
	_CurrentComposeBuffer = Framebuffer::Create(Passes::Color(), Width, Height);
	_TAAComposeBuffer = Framebuffer::Create(Passes::Color(), Width, Height);

	//Present
	_ColorBuffer = Framebuffer::Create(Passes::Color(), Width, Height);
}

double lastReloadShaders = 0.0f;
void WorldRenderer::DrawWorld(float dt, CmdBuffer& cmd, View& view, World& world) {
	PROFILE_FUNC();

	//////////////////
	// Swap Buffers //
	//////////////////
	_LastLightBuffer.state.swap(_TAALightBuffer.state);
	_LastComposeBuffer.state.swap(_TAAComposeBuffer.state);

	// Crtl to reload shaders
	if (Input::IsKeyPressed(Key::LeftControl) && (glfwGetTime()-lastReloadShaders) > 1.0) {
		lastReloadShaders = glfwGetTime();
		
		//TODO: It is a memory leak
		GeometryVoxelPipeline::Get() = GeometryVoxelPipeline();
		GeometrySkyPipeline::Get() = GeometrySkyPipeline();
		OutlineVoxelPipeline::Get() = OutlineVoxelPipeline();
		LightAmbientPipeline::Get() = LightAmbientPipeline();
		LightPointPipeline::Get() = LightPointPipeline();
		LightSpotPipeline::Get() = LightSpotPipeline();
		LightTAAPipeline::Get() = LightTAAPipeline();
		LightBloomStepPipeline::Get() = LightBloomStepPipeline();
		LightBlurPipeline::Get() = LightBlurPipeline();
		LightReflectionPipeline::Get() = LightReflectionPipeline();
		ComposePipeline::Get() = ComposePipeline();
		ComposeTAAPipeline::Get() = ComposeTAAPipeline();
		ColorWorldPipeline::Get() = ColorWorldPipeline();
		//Rest VoxSlot
		world.GetRegistry().view<VoxRenderer>().each([](const entt::entity e, VoxRenderer& vr) {
			vr.VoxSlot = -1;
		});
	}

	////////////
	// Render //
	////////////
	glm::vec2 OFFSETS[16] = {
		glm::vec2(0.5000, 0.3333),
		glm::vec2(0.2500, 0.6667),
		glm::vec2(0.7500, 0.1111),
		glm::vec2(0.1250, 0.4444),
		glm::vec2(0.6250, 0.7778),
		glm::vec2(0.3750, 0.2222),
		glm::vec2(0.8750, 0.5556),
		glm::vec2(0.0625, 0.8889),
		glm::vec2(0.5625, 0.0370),
		glm::vec2(0.3125, 0.3704),
		glm::vec2(0.8125, 0.7037),
		glm::vec2(0.1875, 0.1481),
		glm::vec2(0.6875, 0.4815),
		glm::vec2(0.4375, 0.8148),
		glm::vec2(0.9375, 0.2593),
		glm::vec2(0.0313, 0.5926)
	};

	ViewData viewData;
	{
		viewData.LastViewMatrix = _LastView.ViewMatrix;
		viewData.ViewMatrix = view.ViewMatrix;
		viewData.InverseViewMatrix = glm::inverse(view.ViewMatrix);
		viewData.ProjectionMatrix = view.ProjectionMatrix;
		viewData.InverseProjectionMatrix = glm::inverse(view.ProjectionMatrix);
		viewData.Res = glm::vec2(view.Width, view.Height) * (ENABLE_UPSAMPLING ? 0.5f : 1.0f);
		viewData.iRes = 1.0f / viewData.Res;
		viewData.CameraPosition = view.Position;
		viewData.Jitter = OFFSETS[_Frame%16];
		viewData.Frame = _Frame;
		viewData.ColorTextureRID = _GeometryBuffer.getAttachment(Passes::Geometry_Color).getRID();
		viewData.DepthTextureRID = _GeometryBuffer.getAttachment(Passes::Geometry_Depth).getRID();
		viewData.PalleteColorRID = PalleteCache::GetColorTexture().getRID();
		viewData.PalleteMaterialRID = PalleteCache::GetMaterialTexture().getRID();
		_ViewBuffer.update(&viewData, sizeof(ViewData));
	}

	//Build Voxel Cmds
	{
		PROFILE_SCOPE("Build Voxel Cmds");
		world.GetRegistry().group<VoxRenderer>(entt::get_t<Transform>()).each([&](const entt::entity e, VoxRenderer& v, Transform& t) {
			if (v.Vox.IsValid() && v.Pallete.IsValid()) {
				glm::mat4 mat = t.WorldMatrix;
				mat = glm::translate(mat, -v.Pivot);
				glm::mat4 last_mat = t.PreviousWorldMatrix;
				last_mat = glm::translate(last_mat, -v.Pivot);

				CmdVoxel(mat, last_mat, v.Vox->GetImage(), v.Pallete->GetPalleteIndex());
			}
		});
	}

	
	cmd.timestamp("GBuffer", [&] {
		//G-Buffer
		cmd.use(_GeometryBuffer, [&] {
			GeometrySkyPipeline::Get().Use(cmd, _ViewBuffer, viewData.Res);
			GeometryVoxelPipeline::Get().Use(cmd, _ViewBuffer, viewData.Res, _BlueNoise->GetImage(), Cmds->voxel);
		});
	});

	cmd.timestamp("Outline", [&] {
		//Outline
		cmd.use(_OutlineBuffer, [&] {
			OutlineVoxelPipeline::Get().Use(cmd, _ViewBuffer, _GeometryBuffer, Cmds->outline);
		});
	});

	cmd.timestamp("Lights", [&] {
		//Lights
		cmd.use(_CurrentLightBuffer, [&] {
			LightAmbientPipeline::Get().Use(cmd, _ViewBuffer, _GeometryBuffer, world.ShadowVox->GetVolumeImage(), _BlueNoise->GetImage(), _DefaultSkyBox->GetSkyBox());
			LightPointPipeline::Get().Use(cmd, _ViewBuffer, _GeometryBuffer, world.ShadowVox->GetVolumeImage(), _BlueNoise->GetImage(), _Frame, [&](LightPointPipeline& P) {
				world.GetRegistry().view<const Transform, const Light>().each([&](const entt::entity e, const Transform& t, const Light& l) {
					PROFILE_SCOPE("DrawPointLight()");
					if (l.LightType == Light::Type::Point) {
						P.DrawLight(t.WorldMatrix[3], l.Range, l.Color * l.Intensity, l.Attenuation);
					}
				});
			});
			LightSpotPipeline::Get().Use(cmd, _ViewBuffer, _GeometryBuffer, world.ShadowVox->GetVolumeImage(), _BlueNoise->GetImage(), _Frame, [&](LightSpotPipeline& P) {
				world.GetRegistry().view<const Transform, const Light>().each([&](const entt::entity e, const Transform& t, const Light& l) {
					PROFILE_SCOPE("DrawSpotLight()");
					if (l.LightType == Light::Type::Spot) {
						P.DrawLight(t.WorldMatrix[3], l.Range, l.Color * l.Intensity, l.Attenuation, t.WorldMatrix[2], l.Angle, l.AngleAttenuation);
					}
				});
			});
		});
	});

	cmd.timestamp("LightTAA", [&] {
		//Light TAA
		cmd.use(_TAALightBuffer, [&] {
			LightTAAPipeline::Get().Use(cmd, _ViewBuffer, _BlueNoise->GetImage(), _LastLightBuffer, _CurrentLightBuffer, _GeometryBuffer);
		});
	});

	cmd.timestamp("Reflection", [&] {
		//Reflection
		cmd.use(_ReflectionBuffer, [&] {
			LightReflectionPipeline::Get().Use(cmd, _ViewBuffer, _TAALightBuffer.getAttachment(0), _GeometryBuffer, world.ShadowVox->GetVolumeImage(), _DefaultSkyBox->GetSkyBox(), _BlueNoise->GetImage());
		});
	});

	cmd.timestamp("Bloom", [&] {
		cmd.use(_BloomStepBuffer, [&] {
			LightBloomStepPipeline::Get().Use(cmd, _CurrentLightBuffer.getAttachment(0));
		});
		for (int i = 0; i < BLOOM_MIP_COUNT; i++) {
			cmd.use(_Bloom1Buffer[i], [&] {
				LightBlurPipeline::Get().Use(cmd, (i == 0 ? _BloomStepBuffer : _Bloom2Buffer[i-1]).getAttachment(0), true); // use the step for the first input
			});
			cmd.use(_Bloom2Buffer[i], [&] {
				LightBlurPipeline::Get().Use(cmd, _Bloom1Buffer[i].getAttachment(0), false);
			});
		}
	});

	cmd.timestamp("Compose", [&] {
		//Compose
		cmd.use(_CurrentComposeBuffer, [&] {
			ComposePipeline::Get().Use(cmd, _ViewBuffer, _GeometryBuffer, _TAALightBuffer, _ReflectionBuffer, _Bloom2Buffer);
		});
	});

	cmd.timestamp("ComposeTAA", [&] {
		//Compose TAA
		cmd.use(_TAAComposeBuffer, [&] {
			ComposeTAAPipeline::Get().Use(cmd, _ViewBuffer, _LastComposeBuffer, _CurrentComposeBuffer, _GeometryBuffer);
		});
	});

	cmd.timestamp("Color", [&] {
		//Color (final world color)
		cmd.use(_ColorBuffer, [&] {
			ColorWorldPipeline::Get().Use(cmd, _TAAComposeBuffer.getAttachment(0), _OutlineBuffer);
			//ColorWorldPipeline::Get().Use(cmd, _GeometryBuffer.getAttachment(Passes::Geometry_Color), _OutlineBuffer);
		});
	});
	
	_LastView = view;
	_Frame++;
	if (_Frame > 16 * 100)_Frame = 0;
	Cmds->Clear();
}

Image& WorldRenderer::GetCurrentColorImage() {
	return _ColorBuffer.getAttachment(0);
}

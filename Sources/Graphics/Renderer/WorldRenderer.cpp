#include "WorldRenderer.h"

#include "Core/Engine.h"
#include "Core/Input.h"

#include "Profiler/Profiler.h"
#include "GLFW/glfw3.h"

#include "World/World.h"
#include "World/Components.h"
#include "World/Systems/TransformSystem.h"
#include "World/Systems/CameraSystem.h"
#include "World/Systems/ShadowVoxSystem.h"

#include "Editor/Importer/AssetImporter.h"

#include "Graphics/Graphics.h"
#include "Graphics/BLASManager.h"
// #include "Graphics/Pipelines/GeometrySkyPipeline.h"
//  #include "Graphics/Pipelines/GeometryVoxelPipeline.h"
//  #include "Graphics/Pipelines/LightAmbientPipeline.h"
//  #include "Graphics/Pipelines/LightPointPipeline.h"
//  #include "Graphics/Pipelines/LightSpotPipeline.h"
//  #include "Graphics/Pipelines/LightTAAPipeline.h"
//  #include "Graphics/Pipelines/LightReflectionPipeline.h"
//  #include "Graphics/Pipelines/ComposeTAAPipeline.h"
#include "Graphics/Pipelines/LightBloomStep.h"
#include "Graphics/Pipelines/LightBlur.h"
#include "Graphics/Pipelines/OutlineVoxelPipeline.h"
#include "Graphics/Pipelines/ComposePipeline.h"
#include "Graphics/Pipelines/ColorWorldPipeline.h"
#include "Graphics/Pipelines/PathTracePass.h"
#include "Graphics/Pipelines/GBufferPass.h"
#include "Graphics/Pipelines/DenoiserPass.h"
#include "Graphics/Pipelines/ComposePass.h"

static inline constexpr bool ENABLE_UPSAMPLING = false;

const int BLOOM_MIP_COUNT = 5;
const int MAX_BVH_COUNT = 16384;

struct RenderCmds {
    std::vector<OutlineVoxelPipeline::Cmd> outline;

    void Clear() {
        outline.clear();
    }
};

Image CreateLightImage(uint32 width, uint32 height) {
    return CreateImage({
        .extent = {width, height},
        .format = Format::RGBA16Sfloat,
        .usage = ImageUsage::Sampled | ImageUsage::Storage,
    });
}

Image CreateColorImage(uint32 width, uint32 height) {
    return CreateImage({
        .extent = {width, height},
        .format = Format::BGRA8Unorm,
        .usage = ImageUsage::Sampled | ImageUsage::Attachment | ImageUsage::Storage,
    });
}

WorldRenderer::WorldRenderer() {
    // Create Initial Framebuffers
    RecreateFramebuffer(320, 240);  // Small Initial buffer allocation

    Cmds = new RenderCmds();
    _BlueNoise = Assets::Load("default/LDR_RGBA_0.png");
    _DefaultSkyBox = Assets::Load("default/immenstadter_horn_4k.hdr");
    _ViewBuffer.Build([&](int i) { return CreateBuffer({.size = sizeof(ViewData), .usage = BufferUsage::Storage, .memoryType = MemoryType::CPU_TO_GPU}); });
}

void WorldRenderer::CmdOutline(const glm::mat4& matrix, Image& vox, glm::vec3 color) {
    OutlineVoxelPipeline::Cmd c;
    c.WorldMatrix = matrix;
    c.VolumeRID = GetRID(vox);
    c.Color = color;
    Cmds->outline.push_back(c);
}

void WorldRenderer::RecreateFramebuffer(uint32 Width, uint32 Height) {
    if (Width <= 1) Width = 1;
    if (Height <= 1) Height = 1;

    if (ENABLE_UPSAMPLING) {
        Width /= 2;
        Height /= 2;
    }

    _Bloom1Buffer.clear();
    _Bloom2Buffer.clear();

    // Geometry
    gbuffer = GBuffer(Width, Height);

    // Outline
    _OutlineBuffer = CreateImage({.extent = {Width, Height}, .format = Format::BGRA8Unorm, .usage = ImageUsage::Sampled | ImageUsage::Attachment});

    // Light
    previousLightBuffer = CreateLightImage(Width, Height);
    lightBufferA = CreateLightImage(Width, Height);
    lightBufferB = CreateLightImage(Width, Height);
    _ReflectionBuffer = CreateLightImage(Width, Height);

    _BloomStepBuffer = CreateLightImage(Width >> 1, Height >> 1);  // second mip
    for (int i = 0; i < BLOOM_MIP_COUNT; i++) {
        uint32 mip = 2 + i;  // third mip
        int w = std::max(Width >> mip, 1u);
        int h = std::max(Height >> mip, 1u);
        _Bloom1Buffer.push_back(CreateLightImage(w, h));
        _Bloom2Buffer.push_back(CreateLightImage(w, h));
    }

    if (ENABLE_UPSAMPLING) {
        Width *= 2;
        Height *= 2;
    }

    // Compose
    _LastComposeBuffer = CreateColorImage(Width, Height);
    _CurrentComposeBuffer = CreateColorImage(Width, Height);
    _TAAComposeBuffer = CreateColorImage(Width, Height);

    // Present
    _ColorBuffer = CreateColorImage(Width, Height);

    CmdClear(_ColorBuffer, ClearColor{});
    CmdClear(previousLightBuffer, ClearColor{});
    CmdClear(lightBufferA, ClearColor{});
    CmdClear(lightBufferB, ClearColor{});
    CmdClear(_ReflectionBuffer, ClearColor{});

    CmdClear(_LastComposeBuffer, ClearColor{});
    CmdClear(_CurrentComposeBuffer, ClearColor{});
    CmdClear(_TAAComposeBuffer, ClearColor{});
}

double lastReloadShaders = 0.0f;
void WorldRenderer::DrawWorld(float dt, View& view, World& world) {
    PROFILE_FUNC();
    if (view.Width == 0 || view.Height == 0) return;
    if (timeFlow) {
        timeOfDay += dt * 0.1f;
        if (timeOfDay > 24.0f) {
            timeOfDay = 0.0f;
        }
    }
    if (view.Width != lastView.Width || view.Height != lastView.Height) {
        RecreateFramebuffer(view.Width, view.Height);
    }

    //////////////////
    // Swap Buffers //
    //////////////////
    gbuffer.previousDepth.swap(gbuffer.depth);
    _LastComposeBuffer.swap(_TAAComposeBuffer);

    // Crtl to reload shaders
    if (Input::IsKeyPressed(Key::LeftControl) && (glfwGetTime() - lastReloadShaders) > 1.0) {
        lastReloadShaders = glfwGetTime();

        OutlineVoxelPipeline::Get() = OutlineVoxelPipeline();
        LightBloomStepPipeline::Get() = LightBloomStepPipeline();
        LightBlurPipeline::Get() = LightBlurPipeline();
        ComposePipeline::Get() = ComposePipeline();
        ColorWorldPipeline::Get() = ColorWorldPipeline();
        PathTracePass::Get() = PathTracePass();
        GBufferPass::Get() = GBufferPass();
        DenoiserDiscPass::Get() = DenoiserDiscPass();
        DenoiserAtrousPass::Get() = DenoiserAtrousPass();
        DenoiserTemporalPass::Get() = DenoiserTemporalPass();
        ComposePass::Get() = ComposePass();
        TAAPass::Get() = TAAPass();
        ScreenProbesTracePass::Get() = ScreenProbesTracePass();
        ScreenProbesSamplePass::Get() = ScreenProbesSamplePass();
        ScreenProbesFilterPass::Get() = ScreenProbesFilterPass();
        RadianceProbesTracePass::Get() = RadianceProbesTracePass();
        // Rest VoxSlot
        world.GetRegistry().view<VoxRenderer>().each([](const entt::entity e, VoxRenderer& vr) { vr.VoxSlot = -1; });

        Log::info("Shaders Reloaded!");
    }
    static bool once = true;
    if (Input::IsKeyPressed(Key::P) && (glfwGetTime() - lastReloadShaders) > 1.0 || once) {
        once = false;
        {
            PROFILE_SCOPE("Load Voxel Assets");
            world.GetRegistry().group<VoxRenderer>(entt::get_t<Transform>()).each([&](const entt::entity e, VoxRenderer& v, Transform& t) {
                if (v.Vox.IsValid() && v.Pallete.IsValid()) {
                    glm::mat4 mat = t.WorldMatrix;
                    mat = glm::translate(mat, -v.Pivot);
                    glm::mat4 last_mat = t.PreviousWorldMatrix;
                    last_mat = glm::translate(last_mat, -v.Pivot);

                    v.Vox->GetImage();
                    v.Pallete->GetPalleteIndex();
                }
            });
        }

        lastReloadShaders = glfwGetTime();
        blasInstances.clear();
        voxInstances.clear();
        holdVox.clear();
        world.GetRegistry().group<VoxRenderer>(entt::get_t<Transform>()).each([&](const entt::entity e, VoxRenderer& v, Transform& t) {
            if (v.Vox.IsValid() && v.Pallete.IsValid()) {
                glm::mat4 ttr = t.WorldMatrix;
                ttr = glm::translate(ttr, -v.Pivot);
                ttr = glm::scale(ttr, glm::vec3(0.1f));
                float* tr = (float*)(&ttr);
                BLASManager::VoxBLAS rtData = BLASManager::Get().GetBLAS(v.Vox);
                holdVox.push_back(rtData.geometry);
                rt::BLASInstance inst = {
                    .blas = rtData.blas,
                    .customId = uint32_t(GetRID(rtData.geometry)),
                    .transform = {tr[0], tr[4], tr[8], tr[12], tr[1], tr[5], tr[9], tr[13], tr[2], tr[6], tr[10], tr[14]},
                };
                blasInstances.push_back(inst);
                VoxInstance voxInstance = {
                    .palleteIndex = v.Pallete->GetPalleteIndex(),
                    .geometryBufferRID = GetRID(rtData.geometry),
                };
                voxInstances.push_back(voxInstance);
            }
        });

        // TODO: create voxInstancesBuffer only once
        voxInstancesBuffer = CreateBuffer({
            .name = "Vox Instances",
            .size = voxInstances.size() * sizeof(VoxInstance),
            .usage = BufferUsage::Storage,
            .memoryType = MemoryType::GPU,
        });
        CmdCopy(voxInstances.data(), voxInstancesBuffer, sizeof(VoxInstance) * voxInstances.size());

        tlas = rt::CreateTLAS(blasInstances, false);

        BLASManager::Get().EnsureAllBLASAreBuilt();
        rt::CmdBuildTLAS(tlas);
        Log::info("Acceleration Structure Reloaded!");
    }

    ////////////
    // Render //
    ////////////
    glm::vec2 OFFSETS[16] = {glm::vec2(0.5000, 0.3333), glm::vec2(0.2500, 0.6667), glm::vec2(0.7500, 0.1111), glm::vec2(0.1250, 0.4444), glm::vec2(0.6250, 0.7778), glm::vec2(0.3750, 0.2222), glm::vec2(0.8750, 0.5556), glm::vec2(0.0625, 0.8889),
                             glm::vec2(0.5625, 0.0370), glm::vec2(0.3125, 0.3704), glm::vec2(0.8125, 0.7037), glm::vec2(0.1875, 0.1481), glm::vec2(0.6875, 0.4815), glm::vec2(0.4375, 0.8148), glm::vec2(0.9375, 0.2593), glm::vec2(0.0313, 0.5926)};

    ViewData viewData;
    {
        viewData.LastViewMatrix = lastView.ViewMatrix;
        viewData.ViewMatrix = view.ViewMatrix;
        viewData.InverseViewMatrix = view.CameraMatrix;  // glm::inverse(view.ViewMatrix);
        viewData.ProjectionMatrix = view.ProjectionMatrix;
        viewData.InverseProjectionMatrix = glm::inverse(view.ProjectionMatrix);
        viewData.Res = glm::vec2(view.Width, view.Height) * (ENABLE_UPSAMPLING ? 0.5f : 1.0f);
        viewData.iRes = 1.0f / viewData.Res;
        viewData.CameraPosition = view.Position;
        viewData.Jitter = enableJitter ? OFFSETS[_Frame % 16] : vec2(0.95f, 0.95f);
        viewData.Frame = enablePermutation ? _Frame : 0;
        viewData.ColorTextureRID = GetRID(gbuffer.color);
        viewData.DepthTextureRID = GetRID(gbuffer.depth);
        viewData.PalleteColorRID = GetRID(PalleteCache::GetColorTexture());
        viewData.PalleteMaterialRID = GetRID(PalleteCache::GetMaterialTexture());
        viewData.BlueNoiseRID = GetRID(_BlueNoise->GetImage());
        viewData.TimeOfDay = timeOfDay;
        WriteBuffer(_ViewBuffer, &viewData, sizeof(ViewData));
    }

    if (tlas) {
        CmdTimestamp("Outline", [&] {
            // Outline
            CmdRender({_OutlineBuffer}, {ClearColor{}}, [&] { OutlineVoxelPipeline::Get().Use(_ViewBuffer, gbuffer, Cmds->outline); });
        });
        CmdTimestamp("GBuffer", [&] { GBufferPass::Get().Use(gbuffer, _ViewBuffer, tlas, voxInstancesBuffer); });

        if (enableProbes) {
            // CmdTimestamp("RadianceProbesTrace", [&] { RadianceProbesTracePass::Get().Use(lightBufferA, gbuffer, _ViewBuffer, tlas, voxInstancesBuffer); });

            CmdTimestamp("ProbeTrace", [&] { ScreenProbesTracePass::Get().Use(lightBufferA, gbuffer, _ViewBuffer, tlas, voxInstancesBuffer); });
            if (enableProbesFilter) {
                CmdTimestamp("ProbeFilter", [&] {
                    ScreenProbesFilterPass::Get().Use(lightBufferB, lightBufferA, gbuffer, _ViewBuffer, tlas, voxInstancesBuffer, 1);
                    ScreenProbesFilterPass::Get().Use(lightBufferA, lightBufferB, gbuffer, _ViewBuffer, tlas, voxInstancesBuffer, 0);
                });
                lightBufferA.swap(lightBufferB);
            } else {
                lightBufferA.swap(lightBufferB);
            }
            CmdTimestamp("ProbeSample", [&] { ScreenProbesSamplePass::Get().Use(lightBufferA, lightBufferB, gbuffer, _ViewBuffer, tlas, voxInstancesBuffer); });
            if (enableProbesTemporal) {
                CmdTimestamp("ProbeTemporal", [&] {
                    DenoiserTemporalPass::Get().Use(lightBufferB, lightBufferA, previousLightBuffer, gbuffer, _ViewBuffer);
                    previousLightBuffer.swap(lightBufferB);
                    DenoiserAtrousPass::Get().Use(lightBufferA, previousLightBuffer, gbuffer, _ViewBuffer, 1);
                });
            }
        } else {
            CmdTimestamp("PathTrace", [&] { PathTracePass::Get().Use(lightBufferA, gbuffer, _ViewBuffer, tlas, voxInstancesBuffer); });
            CmdTimestamp("Denoise", [&] {
                if (enableDenoiser) {
                    DenoiserAtrousPass::Get().Use(lightBufferB, lightBufferA, gbuffer, _ViewBuffer, 1);
                    DenoiserTemporalPass::Get().Use(lightBufferA, lightBufferB, previousLightBuffer, gbuffer, _ViewBuffer);
                    previousLightBuffer.swap(lightBufferA);
                    DenoiserAtrousPass::Get().Use(lightBufferB, previousLightBuffer, gbuffer, _ViewBuffer, 2);
                    DenoiserAtrousPass::Get().Use(lightBufferA, lightBufferB, gbuffer, _ViewBuffer, 4);
                    DenoiserAtrousPass::Get().Use(lightBufferB, lightBufferA, gbuffer, _ViewBuffer, 8);
                    DenoiserAtrousPass::Get().Use(lightBufferA, lightBufferB, gbuffer, _ViewBuffer, 16);
                    DenoiserAtrousPass::Get().Use(lightBufferB, lightBufferA, gbuffer, _ViewBuffer, 32);
                    lightBufferA.swap(lightBufferB);
                }
            });
        }

        CmdTimestamp("Compose", [&] { ComposePass::Get().Use(_CurrentComposeBuffer, lightBufferA, gbuffer, _ViewBuffer, voxInstancesBuffer); });
        if (enableTAA) {
            CmdTimestamp("TAA", [&] { TAAPass::Get().Use(_TAAComposeBuffer, _CurrentComposeBuffer, _LastComposeBuffer, gbuffer, _ViewBuffer); });
        } else {
            _TAAComposeBuffer.swap(_CurrentComposeBuffer);
        }
        // CmdTimestamp("Bloom", [&] {
        //     CmdRender({_BloomStepBuffer}, {ClearColor{}}, [&] { LightBloomStepPipeline::Get().Use(lightBufferA); });
        //     for (int i = 0; i < BLOOM_MIP_COUNT; i++) {
        //         CmdRender({_Bloom1Buffer[i]}, {ClearColor{}}, [&] {
        //             LightBlurPipeline::Get().Use((i == 0 ? _BloomStepBuffer : _Bloom2Buffer[i - 1]), true);  // use the step for the first input
        //         });
        //         CmdRender({_Bloom2Buffer[i]}, {ClearColor{}}, [&] { LightBlurPipeline::Get().Use(_Bloom1Buffer[i], false); });
        //     }
        // });
    }
    CmdTimestamp("Color", [&] {
        CmdRender({_ColorBuffer}, {ClearColor{}}, [&] {
            if (outputImage == OutputImage::Composed) {
                ColorWorldPipeline::Get().Use(_TAAComposeBuffer, _OutlineBuffer);
            } else if (outputImage == OutputImage::Diffuse) {
                ColorWorldPipeline::Get().Use(lightBufferB, _OutlineBuffer);
            } else if (outputImage == OutputImage::Normal) {
                ColorWorldPipeline::Get().Use(gbuffer.normal, _OutlineBuffer);
            } else if (outputImage == OutputImage::ScreenProbes) {
                ColorWorldPipeline::Get().Use(lightBufferB, _OutlineBuffer);
            }
            // ColorWorldPipeline::Get().Use(cmd, _GeometryBuffer.getAttachment(Passes::Geometry_Color), _OutlineBuffer);
        });
    });

    lastView = view;
    _Frame++;
    if (_Frame > 16 * 100) _Frame = 0;
    Cmds->Clear();
}

Image& WorldRenderer::GetCurrentColorImage() {
    return _ColorBuffer;
}

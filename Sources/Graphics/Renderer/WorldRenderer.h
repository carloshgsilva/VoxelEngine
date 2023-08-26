#pragma once

#include "Core/Core.h"

#include "World/World.h"

#include "Graphics/Renderer/BVH.h"
#include "Graphics/Renderer/View.h"
#include "Graphics/Canvas.h"

#include "Asset/ImageAsset.h"
#include "Asset/SkyBoxAsset.h"
#include "Shaders/lib/Shared.inl"

#include <glm/vec3.hpp>

// Handles all the state required to Render a World
class WorldRenderer {
    Canvas canvas = {};

    View lastView = {};

    AssetRefT<ImageAsset> _BlueNoise;
    AssetRefT<SkyBoxAsset> _DefaultSkyBox;
    Buffer _ViewBuffer;

    std::vector<Buffer> holdVox;
    Buffer voxInstancesBuffer;
    std::vector<VoxInstance> voxInstances;
    std::vector<rt::BLASInstance> blasInstances;
    rt::TLAS tlas;
    uint32 tlasCount = 0;

    int _Frame{0};

    // Framebuffers
    GBuffer gbuffer;
    Buffer radianceCacheBuffer;

    Image previousLightBuffer;
    Image lightBufferA;
    Image lightBufferB;
    Image specularBufferA;
    Image specularBufferB;
    Image previousSpecularBuffer;

    Image _BloomStepBuffer;
    std::vector<Image> _Bloom1Buffer;
    std::vector<Image> _Bloom2Buffer;

    Image _OutlineBuffer;

    Image _LastComposeBuffer;
    Image _CurrentComposeBuffer;
    Image _TAAComposeBuffer;

    Image _ColorBuffer;

    std::shared_ptr<struct RenderCmds> Cmds;

   public:
    enum class OutputImage : uint {
        Composed,
        Diffuse,
        Normal,
        ReSTIR_GI_Radiance,
        Specular,
    };

    enum class Technique : uint {
        PathTraced,
        ReSTIR,
        IRCache,
    };

    float timeOfDay = 9.0f;
    bool timeFlow = false;
    Technique technique = Technique::ReSTIR;
    OutputImage outputImage = OutputImage::Composed;
    bool enableTAA = true;
    bool enableJitter = true;
    bool enablePermutation = true;
    bool enableDenoiser = true;

    bool enableProbesFilter = false;
    bool enableProbesTemporal = false;

    WorldRenderer();
    ~WorldRenderer() { }

    int CurrentGlyph = 0;
    float fontSize = 14.0f;
    void SelectGlyph(int i);

    void CmdOutline(const glm::mat4& matrix, Image& vox, glm::vec3 color);

    void RecreateFramebuffer(uint32 Width, uint32 Height);

    void DrawWorld(float dt, View& view, World& world);

    Image& GetCurrentColorImage();
};
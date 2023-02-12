#pragma once

#include "Core/Core.h"

#include "World/World.h"

#include "Graphics/Renderer/BVH.h"
#include "Graphics/Renderer/View.h"

#include "Asset/ImageAsset.h"
#include "Asset/SkyBoxAsset.h"
#include "Shaders/lib/Shared.inl"

#include <glm/vec3.hpp>

// Handles all the state required to Render a World
class WorldRenderer {
    View lastView = {};

    AssetRefT<ImageAsset> _BlueNoise;
    AssetRefT<SkyBoxAsset> _DefaultSkyBox;
    PerFrame<Buffer> _ViewBuffer;

    std::vector<Buffer> holdVox;
    Buffer voxInstancesBuffer;
    std::vector<VoxInstance> voxInstances;
    std::vector<rt::BLASInstance> blasInstances;
    rt::TLAS tlas;

    int _Frame{0};

    // Framebuffers
    GBuffer gbuffer;

    Image previousLightBuffer;
    Image lightBufferA;
    Image lightBufferB;
    Image _ReflectionBuffer;

    Image _BloomStepBuffer;
    std::vector<Image> _Bloom1Buffer;
    std::vector<Image> _Bloom2Buffer;

    Image _OutlineBuffer;

    Image _LastComposeBuffer;
    Image _CurrentComposeBuffer;
    Image _TAAComposeBuffer;

    Image _ColorBuffer;

    struct RenderCmds* Cmds;

   public:
    enum class OutputImage : uint {
        Composed,
        Diffuse,
        Normal,
    };

    OutputImage outputImage = OutputImage::Composed;
    bool enableJitter = true;
    bool enablePermutation = true;
    bool enableDenoiser = true;

    WorldRenderer();
    ~WorldRenderer() {
        delete Cmds;
    }

    void CmdOutline(const glm::mat4& matrix, Image& vox, glm::vec3 color);

    void RecreateFramebuffer(uint32 Width, uint32 Height);

    void DrawWorld(float dt, View& view, World& world);

    Image& GetCurrentColorImage();
};
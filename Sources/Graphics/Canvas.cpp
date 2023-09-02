#include "Canvas.h"

#include "Core/Engine.h"
#include "IO/TTFile.h"
#include "Graphics/Pipelines.h"

#include <evk/evk.h>
#include <vector>

using namespace evk;

struct PrimitiveDrawCmd {
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t coordsOffset;
    uint32_t coordsCount;
    glm::vec2 position;
    glm::vec2 scale;
};

struct CanvasImpl {
    TTFFile font;
    Buffer fontCoordsBuffer;
    std::vector<uint64_t> glyphOffsets;
    Buffer vertexBuffer;
    std::vector<glm::vec2> vertices;
    std::vector<PrimitiveDrawCmd> cmds;
};

Canvas::Canvas() {
    impl = new CanvasImpl();
    FileReader fr = FileReader("Assets/Roboto-Medium.ttf");
    impl->font.Load(fr);
    impl->fontCoordsBuffer = CreateBuffer({.size = sizeof(glm::vec2)*65565, .usage = BufferUsage::Storage, .memoryType = MemoryType::GPU});
    impl->vertexBuffer = CreateBuffer({.size = sizeof(glm::vec2)*65565, .usage = BufferUsage::Vertex, .memoryType = MemoryType::GPU});
    
    uint64_t offset = 0ull;
    for(auto& g : impl->font.glyphs) {
        impl->glyphOffsets.push_back(offset/sizeof(glm::vec2));
        uint64_t byteCount = g.coords.size()*sizeof(glm::vec2);
        if(byteCount > 0) {
            CmdCopy((void*)g.coords.data(), impl->fontCoordsBuffer, byteCount, offset);
        }
        offset += byteCount;
    }
}

Canvas::~Canvas() {
    delete impl;
}

void Canvas::LoadFont(const std::string& file) {
}

float Canvas::DrawGlyph(uint32_t g, glm::vec2 position, float size) {
    const auto& glyph = impl->font.glyphs[g];

    PrimitiveDrawCmd cmd;
    cmd.vertexOffset = uint32_t(impl->vertices.size());
    cmd.vertexCount = 6;
    cmd.coordsOffset = impl->glyphOffsets[g];
    cmd.coordsCount = int(glyph.coords.size());
    cmd.position = position;
    cmd.scale = glm::vec2(size);
    
    glm::vec2 gmin = glyph.min - 1.5f/size;
    glm::vec2 gmax = glyph.max + 1.5f/size;
    glm::vec2 p00 = gmin;
    glm::vec2 p01 = glm::vec2(gmin.x, gmax.y);
    glm::vec2 p10 = glm::vec2(gmax.x, gmin.y);
    glm::vec2 p11 = gmax;
    impl->vertices.push_back(p00);
    impl->vertices.push_back(p10);
    impl->vertices.push_back(p01);
    impl->vertices.push_back(p10);
    impl->vertices.push_back(p01);
    impl->vertices.push_back(p11);
    CmdCopy(impl->vertices.data(), impl->vertexBuffer, impl->vertices.size()*sizeof(glm::vec2));
    impl->cmds.push_back(cmd);
    return glyph.advanceWidth*size;
}
float Canvas::DrawText(const char* txt, glm::vec2 position, float size) {
    // TODO: support unicode https://github.com/s22h/cutils/blob/master/src/unicode.h
    int i = 0;
    while(true) {
        uint32_t c = txt[i];
        if(c == 0u)break;
        uint32_t g = impl->font.remap[c];
        position.x += DrawGlyph(g, position, size);
        i++;
    }

    return position.x;
}

void Canvas::Draw(glm::vec2 screenSize) {
    for(auto& cmd : impl->cmds) {
        CmdVertex(impl->vertexBuffer, cmd.vertexOffset*sizeof(glm::vec2));
        GlyphPass::Get().Use(impl->fontCoordsBuffer, screenSize, cmd.position, cmd.scale, float(Engine::GetTime()), cmd.coordsOffset, cmd.coordsCount);
    }
    impl->vertices.clear();
    impl->cmds.clear();
}

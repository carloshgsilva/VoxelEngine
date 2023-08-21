#pragma once

#include <glm/vec2.hpp>
#include <string>

struct Canvas {
    struct CanvasImpl* impl = {};

    Canvas();
    ~Canvas();

    void LoadFont(const std::string& file);
    void DrawGlyph(uint32_t glyph, glm::vec2 position);
    void Draw(glm::vec2 screenSize);
};
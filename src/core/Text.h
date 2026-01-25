#pragma once

#include "core/Shader.h"
#include <glm.hpp>
#include <map>

struct Character {
    unsigned int textureID; // ID handle of texture
    glm::ivec2 size;        // Size of Glyph
    glm::ivec2 bearing;     // Offset from baseline
    unsigned int advance;   // Offset to next Glyph
};

using charMap = std::map<char, Character>;

class Text {
public:
    void initTextVAO(unsigned int* VAO, unsigned int* VBO);
    charMap initFont(const char* font);

    void renderText(Shader& s, unsigned int VAO, unsigned int VBO, std::string text,
        float x, float y, float scale, glm::vec3 color, charMap characters);
};

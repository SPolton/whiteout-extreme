#pragma once

#include "core/Shader.h"
#include "components/Model.h"
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
    Text();
    Text(std::string vertexPath, std::string fragmentPath);

    void update();
    void update(unsigned int frameCount);

    void initTextVAO(unsigned int* VAO, unsigned int* VBO);
    charMap initFont(const char* font);

    void renderText(Shader& s, unsigned int VAO, unsigned int VBO, std::string text,
        float x, float y, float scale, glm::vec3 color, charMap characters);

private:
    Shader textShader;
    charMap characters;
    Model textModel;

    unsigned int textVAO = 0;
    unsigned int textVBO = 0;
};

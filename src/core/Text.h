#pragma once

#include "core/render/ShaderProgram.hpp"
#include "core/gl/GLHandles.hpp"
#include <glm/glm.hpp>
#include <map>

struct Character {
    TextureHandle textureID; // ID handle of texture
    glm::ivec2 size;         // Size of Glyph
    glm::ivec2 bearing;      // Offset from baseline
    unsigned int advance;    // Offset to next Glyph
};

using charMap = std::map<char, Character>;

struct TextPosition {
    float x;
    float y;
    float scale;
};

class Text {
public:
    Text();
    Text(const std::string& vertexPath, const std::string& fragmentPath);

    void beginText();
    void endText();
    void setProjection(float width, float height);
    void renderText(const std::string& text, TextPosition pos, glm::vec3 color);
    void loadFont(const std::string& fontName, int size);

private:
    ShaderProgram textShader;
    charMap characters;
    
    VertexArrayHandle textVAO;
    VertexBufferHandle textVBO;
    
    charMap initFont(const char* fontName, int size);
};

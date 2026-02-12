#include "Text.h"

#include <ft2build.h>
#include <glm/gtc/type_ptr.hpp>
#include FT_FREETYPE_H
#include "utils/logger.h"

Text::Text()
    : Text("assets/shaders/shader_text.vert", "assets/shaders/shader_text.frag")
{
}

Text::Text(const std::string& vertexPath, const std::string& fragmentPath)
    : textShader(vertexPath, fragmentPath)
{
    // default use arial
    characters = initFont("assets/fonts/Arial.ttf", 48);
    
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    logger::info("Text renderer initialized with {} characters", characters.size());
}

charMap Text::initFont(const char* fontName, int size) {
    charMap Characters;
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        logger::error("FREETYPE: Could not init FreeType Library");
        return Characters;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontName, 0, &face))
    {
        logger::error("FREETYPE: Failed to load font: {}", fontName);
        FT_Done_FreeType(ft);
        return Characters;
    }
    
    FT_Set_Pixel_Sizes(face, 0, size);
    
    GLint previousUnpackAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousUnpackAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //Initialize characters
    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            logger::warn("FREETYPE: Failed to load Glyph: {}", c);
            continue;
        }
        
        TextureHandle texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        Characters.emplace(
            c,
            Character{
                std::move(texture),
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            }
        );
    }
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, previousUnpackAlignment);
    glBindTexture(GL_TEXTURE_2D, 0);

    return Characters;
}

void Text::setProjection(float width, float height)
{
    glm::mat4 projection = glm::ortho(0.0f, width, 0.0f, height);
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUseProgram(0);
}

void Text::renderText(const std::string& text, TextPosition pos, glm::vec3 color)
{
    // activate corresponding render state	
    textShader.use();
    glUniform3f(glGetUniformLocation(textShader, "textColor"), color.x, color.y, color.z);
    glUniform1i(glGetUniformLocation(textShader, "text"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);
    
    // iterate through all characters
    for (char c : text)
    {
        const Character& ch = characters[c];

        float xpos = pos.x + ch.bearing.x * pos.scale;
        float ypos = pos.y - (ch.size.y - ch.bearing.y) * pos.scale;

        float w = ch.size.x * pos.scale;
        float h = ch.size.y * pos.scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        pos.x += (ch.advance >> 6) * pos.scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void Text::beginText() {
    textShader.use();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
}

void Text::endText() {
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);
}

void Text::loadFont(const std::string& fontName, int size)
{
    std::string fontPath = "assets/fonts/" + fontName;
    characters = initFont(fontPath.c_str(), size);
}

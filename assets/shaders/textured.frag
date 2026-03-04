// https://learnopengl.com/Getting-started/Textures

#version 330 core

uniform sampler2D baseColorTexture;
uniform vec2 textureScrollOffset;

in vec2 texCoord;
out vec4 fragColor;

void main()
{
    // Apply scroll offset and use fract() to wrap UVs for seamless tiling
    vec2 rollingUV = fract(texCoord + textureScrollOffset);
    fragColor = texture(baseColorTexture, rollingUV);
}

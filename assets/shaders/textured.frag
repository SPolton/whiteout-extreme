// https://learnopengl.com/Getting-started/Textures

#version 330 core

uniform sampler2D baseColorTexture;
uniform sampler2D overlayColorTexture;

in vec3 outColor;
in vec2 texCoord;
out vec4 fragColor;

void main()
{
	fragColor = texture(baseColorTexture, texCoord);
}

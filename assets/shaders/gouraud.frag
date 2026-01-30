//
// https://learnopengl.com/Getting-started/Textures
// https://learnopengl.com/Lighting/Basic-Lighting
//

#version 330 core

uniform sampler2D baseColorTexture;
uniform sampler2D overlayColorTexture;
// uniform sampler2D specTexture;

in vec2 texCoord;
in vec3 lightingResult;

out vec4 fragColor;

void main()
{
	fragColor = vec4(lightingResult, 1.0) * texture(baseColorTexture, texCoord);
}

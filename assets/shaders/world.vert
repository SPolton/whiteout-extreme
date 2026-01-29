//
// https://learnopengl.com/Getting-started/Textures
// https://learnopengl.com/Lighting/Basic-Lighting
//

#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inUV;

out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;

uniform mat4 normalMatrix;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(inPosition, 1.0);
	fragPos = vec3(model * vec4(inPosition, 1.0)); // The actual fragment's position

	texCoord = inUV;
	normal = vec3(normalMatrix * vec4(inNormal, 1.0));
}

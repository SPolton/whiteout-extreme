#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in float aSize;
layout (location = 2) in float aLife;

uniform mat4 view;
uniform mat4 projection;

out float vLife;

void main()
{
    vLife = aLife;
    gl_Position = projection * view * vec4(aPosition, 1.0);
    gl_PointSize = max(aSize, 1.0);
}

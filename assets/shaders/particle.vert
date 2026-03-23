#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in float aSize;
layout (location = 2) in float aLife;

uniform mat4 view;
uniform mat4 projection;
uniform float minPx;
uniform float maxPx;

out float vLife;

void main()
{
    vLife = aLife;
    vec4 viewPos = view * vec4(aPosition, 1.0);
    gl_Position = projection * viewPos;

    float dist = max(-viewPos.z, 0.01);
    float perspectiveScale = projection[1][1];
    float scaleConstant = 1.0;

    float size = (aSize * perspectiveScale * scaleConstant) / dist;

    // Fall back to sane defaults since OpenGL defaults to 0.0
    float resolvedMinPx = (minPx > 0.0) ? minPx : 1.0;
    float resolvedMaxPx = (maxPx > 0.0) ? maxPx : 100.0;
    if (resolvedMaxPx < resolvedMinPx) {
        resolvedMaxPx = resolvedMinPx;
    }

    gl_PointSize = clamp(size, resolvedMinPx, resolvedMaxPx);
}

#version 460 core

in float vLife;
out vec4 FragColor;

void main()
{
    vec2 center = gl_PointCoord - vec2(0.5);
    float radius = length(center);
    if (radius > 0.5) {
        discard;
    }

    float softEdge = smoothstep(0.5, 0.05, radius);
    float lifeAlpha = clamp(vLife, 0.0, 1.0);

    vec3 color = vec3(0.95, 0.97, 1.0);
    FragColor = vec4(color, softEdge * lifeAlpha);
}

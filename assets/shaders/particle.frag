#version 460 core

in float vLife;
in vec2 vUv;
in vec3 vColor;
out vec4 FragColor;

void main()
{
    vec2 center = vUv - vec2(0.5);
    float radius = length(center);
    if (radius > 0.5) {
        discard;
    }

    float softEdge = smoothstep(0.5, 0.05, radius);
    float lifeAlpha = clamp(vLife, 0.0, 1.0);

    FragColor = vec4(vColor, softEdge * lifeAlpha);
}

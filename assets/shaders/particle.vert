#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in float aSize;
layout (location = 2) in float aLife;

uniform mat4 view;
uniform mat4 projection;

out float vLife;
out vec2 vUv;

void main()
{
    vLife = aLife;
    const vec2 quadCorners[6] = vec2[](
        vec2(-0.5, -0.5),
        vec2( 0.5, -0.5),
        vec2( 0.5,  0.5),
        vec2(-0.5, -0.5),
        vec2( 0.5,  0.5),
        vec2(-0.5,  0.5)
    );
    vec2 corner = quadCorners[gl_VertexID % 6];
    vUv = corner + vec2(0.5);

    mat3 invViewRot = transpose(mat3(view));
    vec3 cameraRight = normalize(invViewRot[0]);
    vec3 cameraUp = normalize(invViewRot[1]);
    vec3 worldOffset = (cameraRight * corner.x + cameraUp * corner.y) * aSize;

    vec3 worldPos = aPosition + worldOffset;
    gl_Position = projection * view * vec4(worldPos, 1.0);
}

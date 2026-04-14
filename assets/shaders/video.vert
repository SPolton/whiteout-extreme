#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in vec2 aTex; // Ton slot d'UV

out vec2 TexCoords;

void main() {
    TexCoords = vec2(aTex.x, 1.0 - aTex.y); 
    
    gl_Position = vec4(aPos, 1.0);
}

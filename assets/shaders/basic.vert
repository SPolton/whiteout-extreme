#version 330 core

layout (location = 0) in vec3 positionIn;
layout (location = 3) in vec2 uvIn;

out vec2 uvOut;

uniform mat4 model;

void main() {
	uvOut = uvIn;
	gl_Position = model * vec4(positionIn, 1.0);
}

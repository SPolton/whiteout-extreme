//
// https://learnopengl.com/Lighting/Basic-Lighting
// https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/2.5.basic_lighting_exercise3/basic_lighting_exercise3.cpp
//

#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inUV;

out vec2 texCoord;
out vec3 lightingResult; // resulting color from lighting calculations

uniform vec3 lightColor;
uniform vec3 lightPos;

uniform vec3 viewPos;
uniform float shininess;

uniform float ambientStrength;
uniform float specularStrength;

uniform mat4 normalMatrix;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(inPosition, 1.0);

	texCoord = inUV;

	// Gouraud shading
    vec3 position = vec3(model * vec4(inPosition, 1.0));
    vec3 normal = vec3(normalMatrix * vec4(inNormal, 1.0));
    
    // Ambient lighting
    vec3 ambient = ambientStrength * lightColor;
  	
    // Diffuse lighting
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - position);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular lighting
    vec3 viewDir = normalize(viewPos - position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    lightingResult = ambient + diffuse + specular;
}

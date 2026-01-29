//
// https://learnopengl.com/Getting-started/Textures
// https://learnopengl.com/Lighting/Basic-Lighting
//

#version 330 core

uniform sampler2D baseColorTexture;
uniform sampler2D overlayColorTexture;
// uniform sampler2D specTexture;

uniform vec3 lightColor;
uniform vec3 lightPos;

uniform vec3 viewPos;
uniform float shininess;

uniform float ambientStrength;
uniform float specularStrength;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

out vec4 fragColor;

void main()
{
	// Ambient lighting
    vec3 ambient = ambientStrength * lightColor;

	// Diffuse lighting
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// Specular mapping
	// vec4 specularColor = texture(specMapping, TexCoords);

	// Specular Lighting
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = specularStrength * spec * lightColor;

	// Combined (Phong) lighting
    vec4 light = vec4(ambient + diffuse + specular, 1.0);

	fragColor = light * texture(baseColorTexture, texCoord);
}

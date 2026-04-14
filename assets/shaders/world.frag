//
// https://learnopengl.com/Getting-started/Textures
// https://learnopengl.com/Lighting/Basic-Lighting
//

#version 330 core

uniform sampler2D baseColorTexture;
uniform sampler2D overlayColorTexture;
uniform sampler2D shadowMap;
// uniform sampler2D specTexture;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightDir;

uniform vec3 viewPos;
uniform float shininess;
uniform bool directionalLight;
uniform bool shadowsEnabled;

uniform float ambientStrength;
uniform float specularStrength;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;
in vec4 fragPosLightSpace;

out vec4 fragColor;

const float kProjCoordScale = 0.5;
const float kProjCoordBias = 0.5;
const float kShadowFarClip = 1.0;
const float kShadowSlopeBias = 0.05;
const float kShadowMinBias = 0.005;
const int kShadowPcfRadius = 1;
const float kShadowPcfSampleCount = float((2 * kShadowPcfRadius + 1) * (2 * kShadowPcfRadius + 1));
const float kNoShadow = 0.0;
const float kFullLight = 1.0;

float calculateShadow(vec4 lightSpacePos, vec3 surfaceNormal, vec3 lightDir)
{
	// perform perspective divide
	vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
	// transform to [0,1] range
	projCoords = projCoords * kProjCoordScale + kProjCoordBias;

	// keep shadow disabled outside the light frustum far clip
	if (projCoords.z > kShadowFarClip)
		return kNoShadow;

	// calculate bias (based on depth map resolution and slope)
	float bias = max(
		kShadowSlopeBias * (kFullLight - dot(surfaceNormal, lightDir)),
		kShadowMinBias
	);

	// PCF
	vec2 texelSize = kFullLight / textureSize(shadowMap, 0);

	float shadow = kNoShadow;
	for (int x = -kShadowPcfRadius; x <= kShadowPcfRadius; ++x) {
		for (int y = -kShadowPcfRadius; y <= kShadowPcfRadius; ++y) {
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += (projCoords.z - bias) > pcfDepth ? kFullLight : kNoShadow;
		}
	}

	return shadow / kShadowPcfSampleCount;
}

void main()
{
	// Ambient lighting
    vec3 ambient = ambientStrength * lightColor;

	// Diffuse lighting
	vec3 norm = normalize(normal);
	vec3 toLightDir = directionalLight
		? normalize(-lightDir)
		: normalize(lightPos - fragPos);
	float diff = max(dot(norm, toLightDir), kNoShadow);
	vec3 diffuse = diff * lightColor;

	// Specular mapping
	// vec4 specularColor = texture(specMapping, TexCoords);

	// Specular Lighting
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-toLightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), kNoShadow), shininess);
	vec3 specular = specularStrength * spec * lightColor;

	// Combined (Phong) lighting
	float shadow = shadowsEnabled ? calculateShadow(fragPosLightSpace, norm, toLightDir) : kNoShadow;
	vec4 light = vec4(ambient + (kFullLight - shadow) * (diffuse + specular), kFullLight);

	fragColor = light * texture(baseColorTexture, texCoord);
}

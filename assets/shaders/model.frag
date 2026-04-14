#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 lightDir;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform bool directionalLight;
uniform bool shadowsEnabled;
uniform float shadowNearPlane;
uniform float shadowFarPlane;

const float kProjCoordScale = 0.5;
const float kProjCoordBias = 0.5;
const float kShadowFarClip = 1.0;
const float kShadowSlopeBias = 0.05;
const float kShadowMinBias = 0.005;
const int kShadowPcfRadius = 1;
const float kShadowPcfSampleCount = float((2 * kShadowPcfRadius + 1) * (2 * kShadowPcfRadius + 1));
const float kAmbientStrength = 0.3;
const float kSpecularShininess = 32.0;
const vec3 kSpecularLumaWeights = vec3(0.2126, 0.7152, 0.0722);
const float kNoShadow = 0.0;
const float kFullLight = 1.0;

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * kProjCoordScale + kProjCoordBias;

    // keep shadow disabled outside the light frustum far clip
    if (projCoords.z > kShadowFarClip)
        return kNoShadow;

    // calculate bias (based on depth map resolution and slope)
    float bias = max(
        kShadowSlopeBias * (kFullLight - dot(normal, lightDir)),
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
    // ambient
    vec3 ambient = kAmbientStrength * texture(texture_diffuse1, TexCoords).rgb;
  	
    // diffuse
    vec3 norm = normalize(Normal);
    vec3 toLightDir = directionalLight
        ? normalize(-lightDir)
        : normalize(lightPos - FragPos);
    float diff = max(dot(norm, toLightDir), kNoShadow);
    vec3 diffuse = diff * lightColor * texture(texture_diffuse1, TexCoords).rgb;
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-toLightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), kNoShadow), kSpecularShininess);
    float specularMask = dot(texture(texture_specular1, TexCoords).rgb, kSpecularLumaWeights);
    vec3 specular = spec * lightColor * vec3(specularMask);

    // calculate shadow
    float shadow = shadowsEnabled ? calculateShadow(FragPosLightSpace, norm, toLightDir) : kNoShadow;

    // combine lighting
    vec3 result = ambient + (kFullLight - shadow) * (diffuse + specular);
    FragColor = vec4(result, kFullLight);
}

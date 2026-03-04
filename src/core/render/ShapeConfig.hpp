namespace render
{

// Configuration structs for entity creation
struct SphereConfig {
    float radius = 1.0f;
    int slices = 16;
    int stacks = 16;
    bool isSkybox = false;
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::quat rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    GLenum textureWrapMode = GL_CLAMP_TO_EDGE;
    GLenum textureFilterMode = GL_LINEAR;
};

struct PlaneConfig {
    float size = 10.0f;
    float uvRepeat = 1.0f;  // Set to > 1.0 for tiling effect
    bool isInfinite = false;  // If true, uses infinitePlaneSize with repeating UVs
    float infinitePlaneSize = 10000.0f;  // Size for infinite planes
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::quat rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    GLenum textureWrapMode = GL_CLAMP_TO_EDGE;
    GLenum textureFilterMode = GL_LINEAR;
};

struct ModelConfig {
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::quat rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
};

} // namespace render

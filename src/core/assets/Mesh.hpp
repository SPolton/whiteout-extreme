#pragma once

#include "Texture.hpp"
#include "core/buffer/VertexBuffer.hpp"
#include "core/render/ShaderProgram.hpp"
#include "core/gl/GLHandles.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>
#include <memory>

#define MAX_BONE_INFLUENCE 4

struct MeshVertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct MeshTextureInfo {
    std::shared_ptr<Texture> texture;
    std::string type;
};

class Mesh {
public:
    // mesh Data
    std::vector<MeshVertex>           vertices;
    std::vector<unsigned int>     indices;
    std::vector<MeshTextureInfo>  textures;

    // constructor
    Mesh(std::vector<MeshVertex> vertices, std::vector<unsigned int> indices, std::vector<MeshTextureInfo> textures);

    // render the mesh
    void draw(ShaderProgram& shader);

private:
    // render data 
    VertexArrayHandle VAO;
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;

    // initializes all the buffer objects/arrays
    void setupMesh();
};

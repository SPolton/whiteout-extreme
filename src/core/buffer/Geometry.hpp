#pragma once

#include "VertexArray.hpp"
#include "VertexBuffer.hpp"

#include <glm/glm.hpp>
#include <vector>

using Position = glm::vec3;
using Color = glm::vec3;
using Normal = glm::vec3;
using UV = glm::vec2;
using Index = uint32_t;

// List of vertices and texture coordinates using std::vector and glm::vec3
struct CPU_Geometry {
	std::vector<Position> positions;
    std::vector<Color> colors;
	std::vector<Normal> normals;
    std::vector<UV> uvs;             // You need the uv for texture mapping
    std::vector<Index> indices;      // Index buffer (EBO) is needed for the bonuses
};


// VAO and two VBOs for storing vertices and texture coordinates, respectively
class GPU_Geometry {
public:

	GPU_Geometry();
	// Public interface
	void bind() {
		vao.bind();
	}

    void Update(CPU_Geometry const& data);
    // for loading pictures in menu screens, only need index position and uv
    void Update2D(CPU_Geometry const& data);

private:
    // note: due to how OpenGL works, vao needs to be
    // defined and initialized before the vertex buffers
    VertexArray vao;
    VertexBuffer positionsBuffer;
    VertexBuffer colorsBuffer;
    VertexBuffer normalsBuffer;
    VertexBuffer uvsBuffer;
    IndexBuffer indexBuffer;

	void UpdatePositions(size_t count, Position const * positions);
    void UpdateColors(size_t count, Color const * colors);
    void UpdateNormals(size_t count, Normal const * normals);
	void UpdateUVs(size_t count, UV const * uvs);
    void UpdateIndices(size_t count, Index const * indices);
};

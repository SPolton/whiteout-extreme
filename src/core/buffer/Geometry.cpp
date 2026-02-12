#include "Geometry.hpp"

GPU_Geometry::GPU_Geometry()
    : vao()
    , positionsBuffer(0, sizeof(Position) / sizeof(float), GL_FLOAT)
    , colorsBuffer(1, sizeof(Color) / sizeof(float), GL_FLOAT)
    , normalsBuffer(2, sizeof(Normal) / sizeof(float), GL_FLOAT)
    , uvsBuffer(3, sizeof(UV) / sizeof(float), GL_FLOAT)
    , indexBuffer(4, sizeof(Index) / sizeof(float), GL_FLOAT)
{
    // Bind VAO first, then setup all vertex attributes
    vao.bind();
    positionsBuffer.setupAttribute();
    colorsBuffer.setupAttribute();
    normalsBuffer.setupAttribute();
    uvsBuffer.setupAttribute();
    // Index buffer doesn't need attribute setup
    glBindVertexArray(0); // Unbind VAO
}


void GPU_Geometry::UpdatePositions(size_t const count, Position const *positions)
{
    positionsBuffer.uploadData(sizeof(Position) * count, positions, GL_STATIC_DRAW);
}

void GPU_Geometry::UpdateColors(size_t const count, Color const *colors)
{
    colorsBuffer.uploadData(sizeof(Color) * count, colors, GL_STATIC_DRAW);
}

void GPU_Geometry::UpdateNormals(size_t const count, Normal const *normals)
{
    normalsBuffer.uploadData(sizeof(Normal) * count, normals, GL_STATIC_DRAW);
}

void GPU_Geometry::UpdateUVs(size_t const count, UV const * uvs)
{
    uvsBuffer.uploadData(sizeof(UV) * count, uvs, GL_STATIC_DRAW);
}

void GPU_Geometry::UpdateIndices(size_t const count, Index const *indices)
{
    // Must bind VAO before uploading index data
    // Element array buffer binding is part of VAO state
    vao.bind();
    indexBuffer.uploadData(sizeof(Index) * count, indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
}


void GPU_Geometry::Update(CPU_Geometry const &data)
{
    // Sanity check to make sure the positions, normals and color have equal sizes.
    assert(data.positions.size() == data.normals.size());
    assert(data.positions.size() == data.colors.size());
    // assert(data.positions.size() == data.uvs.size());

    UpdatePositions(data.positions.size(), data.positions.data());
    UpdateColors(data.colors.size(), data.colors.data());
    UpdateNormals(data.normals.size(), data.normals.data());
    UpdateUVs(data.uvs.size(), data.uvs.data());
    UpdateIndices(data.indices.size(), data.indices.data());
}

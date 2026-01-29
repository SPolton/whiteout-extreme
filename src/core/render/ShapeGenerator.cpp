#include "ShapeGenerator.hpp"

#include <glm/ext/matrix_transform.hpp>

// Returns an index vector where every 3 indices form a triangle.
// https://songho.ca/opengl/gl_sphere.html#sphere
std::vector<Index> getTriangleIndices(int const slices, int const stacks)
{
    // generate CCW index list of sphere triangles
    // k1--k1+1
    // |  / |
    // | /  |
    // k2--k2+1
    std::vector<Index> indices;
    Index k1, k2;
    for(int i = 0; i < stacks; ++i)
    {
        k1 = i * (slices + 1);     // beginning of current stack
        k2 = k1 + slices + 1;      // beginning of next stack

        for(int j = 0; j < slices; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
    return indices;
}

// Returns all points, normals, and UVs of the sphere with +Y as up.
// Surface of revolution (SOR) is used to generate the positions.
// https://songho.ca/opengl/gl_sphere.html#sphere
CPU_Geometry SpherePointsSOR(float const radius, int const slices, int const stacks)
{
    CPU_Geometry geom{};
    
    Position pos;
    float xz;                         // vertex position
    float lengthInv = 1.0f / radius;  // vertex normal
    float s, t;                       // vertex texCoord

    float sectorStep = glm::two_pi<float>() / slices;
    float stackStep = glm::pi<float>() / stacks;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stacks; ++i)
    {
        // For some reason, need to generate in reverse 
        stackAngle = -glm::pi<float>() / 2 + i * stackStep; // starting from -pi/2 to pi/2
        xz = radius * cosf(stackAngle);     // r * cos(u)
        pos.y = radius * sinf(stackAngle);  // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= slices; ++j)
        {
            sectorAngle = j * sectorStep;  // starting from 0 to 2pi

            // vertex position (x, y, z)
            pos.x = xz * cosf(sectorAngle);  // r * cos(u) * cos(v)
            pos.z = xz * sinf(sectorAngle);  // r * cos(u) * sin(v)

            geom.positions.push_back(pos);

            // normalized vertex normal (nx, ny, nz)
            Normal norm = pos * lengthInv;
            geom.normals.push_back(norm);

            // vertex tex coord (s, t) range between [0, 1]
            s = 1.0f - (float)j / slices; // Flip since stackAngle is in reverse
            t = (float)i / stacks;

            UV mapping = UV(s, t);
            geom.uvs.push_back(mapping);
        }
    }
    return geom;
}

// Returns the completed geometry for a SOR sphere, with the up axis in the +Y direction.
CPU_Geometry ShapeGenerator::Sphere(float const radius, int const slices, int const stacks)
{
    CPU_Geometry geom = SpherePointsSOR(radius, slices, stacks);
    geom.colors = std::vector<Color>(geom.positions.size(), {1.f, 1.f, 0.f});
    geom.indices = getTriangleIndices(slices, stacks);

    return geom;
}

//======================================================================================================================

static void colouredTriangles(CPU_Geometry &geom, glm::vec3 col);
static void positiveZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void positiveXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void negativeZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void negativeXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void positiveYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void negativeYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);

// Returns a 1x1x1 cube with different color sides.
CPU_Geometry ShapeGenerator::UnitCube()
{
    std::vector<glm::vec3> originQuad{};
    originQuad.emplace_back(-0.5, 0.5, 0.0); // top-left
    originQuad.emplace_back(-0.5, -0.5, 0.0); // bottom-left
    originQuad.emplace_back(0.5, 0.5, 0.0); // top-right

    originQuad.emplace_back(-0.5, -0.5, 0.0); // bottom-left
    originQuad.emplace_back(0.5, -0.5, 0.0); // bottom-right
    originQuad.emplace_back(0.5, 0.5, 0.0); // top-right

    CPU_Geometry square{};

    positiveZFace(originQuad, square);
    colouredTriangles(square, {1.f, 1.f, 0.f});

    positiveXFace(originQuad, square);
    colouredTriangles(square, {1.f, 0.f, 0.f});

    negativeZFace(originQuad, square);
    colouredTriangles(square, {0.f, 1.f, 0.f});

    negativeXFace(originQuad, square);
    colouredTriangles(square, {0.f, 0.f, 1.f});

    positiveYFace(originQuad, square);
    colouredTriangles(square, {1.f, 0.f, 1.f});

    negativeYFace(originQuad, square);
    colouredTriangles(square, {0.f, 1.f, 1.f});

    return square;
}

void colouredTriangles(CPU_Geometry &geom, glm::vec3 col)
{
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
}

void positiveZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.5));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(T * glm::vec4((*i), 1.0));
    }
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
}

void positiveXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
}

void negativeZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
}

void negativeXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
}

void positiveYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
}

void negativeYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
}

CPU_Geometry ShapeGenerator::Triangle()
{
    CPU_Geometry geom{};
    
    geom.positions = {
        glm::vec3(-0.5f, -0.5f, 0.0f),
        glm::vec3( 0.5f, -0.5f, 0.0f),
        glm::vec3( 0.0f,  0.5f, 0.0f)
    };
    
    geom.colors = {
        glm::vec3(1.0f, 0.0f, 0.0f),  // Red
        glm::vec3(0.0f, 1.0f, 0.0f),  // Green
        glm::vec3(0.0f, 0.0f, 1.0f)   // Blue
    };
    
    geom.normals = {
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };
    
    geom.uvs = {
        glm::vec2(0.0f, 0.0f),  // Bottom-left
        glm::vec2(1.0f, 0.0f),  // Bottom-right
        glm::vec2(0.5f, 1.0f)   // Top-center
    };
    
    return geom;
}

CPU_Geometry ShapeGenerator::Square()
{
    CPU_Geometry geom{};
    
    geom.positions = {
        glm::vec3(-0.5f, -0.5f, 0.0f),  // Bottom-left
        glm::vec3( 0.5f, -0.5f, 0.0f),  // Bottom-right
        glm::vec3( 0.5f,  0.5f, 0.0f),  // Top-right
        glm::vec3( 0.5f,  0.5f, 0.0f),  // Top-right
        glm::vec3(-0.5f,  0.5f, 0.0f),  // Top-left
        glm::vec3(-0.5f, -0.5f, 0.0f)   // Bottom-left
    };
    
    geom.colors = {
        glm::vec3(1.0f, 0.0f, 0.0f),  // Red
        glm::vec3(0.0f, 1.0f, 0.0f),  // Green
        glm::vec3(0.0f, 0.0f, 1.0f),  // Blue
        glm::vec3(0.0f, 0.0f, 1.0f),  // Blue
        glm::vec3(1.0f, 1.0f, 0.0f),  // Yellow
        glm::vec3(1.0f, 0.0f, 0.0f)   // Red
    };
    
    geom.normals = {
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };
    
    geom.uvs = {
        glm::vec2(0.0f, 0.0f),  // Bottom-left
        glm::vec2(1.0f, 0.0f),  // Bottom-right
        glm::vec2(1.0f, 1.0f),  // Top-right
        glm::vec2(1.0f, 1.0f),  // Top-right
        glm::vec2(0.0f, 1.0f),  // Top-left
        glm::vec2(0.0f, 0.0f)   // Bottom-left
    };
    
    return geom;
}

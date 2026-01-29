#include "math.hpp"

#include <glm/ext/matrix_transform.hpp>

namespace math::random
{
    float RandomFloat(float const min, float const max)
    {
        std::random_device rd; // Obtain a random number from hardware
        std::mt19937 gen(rd()); // Seed the generator

        std::uniform_real_distribution<float> dist(min, max); // Define range

        return dist(gen); // Generate number
    }

    int RandomInt(int const min, int const max)
    {
        std::random_device rd; // Obtain a random number from hardware
        std::mt19937 gen(rd()); // Seed the generator

        std::uniform_int_distribution<int> dist(min, max); // Define range

        return dist(gen); // Generate number
    }

} // namespace math::random


namespace math::transform
{
    glm::mat4 TranslationToMatrix(glm::vec3 const & translation)
    {
        return glm::translate(glm::identity<glm::mat4>(), translation);
    }

    glm::mat4 RotationToMatrix(glm::vec3 const & eulerAngles)
    {
        glm::mat4 const yawMatrix = glm::rotate(glm::identity<glm::mat4>(), eulerAngles.x, RightVec3);
        glm::mat4 const pitchMatrix = glm::rotate(glm::identity<glm::mat4>(), eulerAngles.y, UpVec3);
        glm::mat4 const rollMatrix = glm::rotate(glm::identity<glm::mat4>(), eulerAngles.z, ForwardVec3);
        return yawMatrix * pitchMatrix * rollMatrix;
    }

    glm::mat4 ScaleToMatrix(glm::vec3 const & scale)
    {
        return glm::scale(glm::identity<glm::mat4>(), scale);
    }

} // namespace math::transform

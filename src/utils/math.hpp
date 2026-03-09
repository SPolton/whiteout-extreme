#pragma once

#include <algorithm>  // For std::shuffle
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace math {
    template<typename T>
    [[nodiscard]]
    T Lerp(T first, T second, T t)
    {
        return first + (second - first) * t;
    }

    template<typename T>
    [[nodiscard]]
    T PingPong(T time, T duration)
    {
        return duration - std::abs(std::fmod(time, (duration * 2.0f)) - duration);
    }

} // namespace math


namespace math::random {
    // Min, Max inclusive
    [[nodiscard]] float RandomFloat(float min, float max);

    // Min, Max inclusive
    [[nodiscard]] int RandomInt(int min, int max);

    template <typename T>
    void Shuffle(T* values, size_t const count)
    {
        std::random_device rd;   // Obtain a random seed
        std::mt19937 gen(rd());  // Standard Mersenne Twister generator
        std::shuffle(values, values + count, gen); // Shuffle the vector
    }

} // namespace math::random


namespace math::transform {
    inline static constexpr glm::vec3 RightVec3{ 1.0f, 0.0f, 0.0f };
    inline static constexpr glm::vec3 UpVec3{ 0.0f, 1.0f, 0.0f };
    inline static constexpr glm::vec3 ForwardVec3{ 0.0f, 0.0f, 1.0f };

    glm::mat4 TranslationToMatrix(glm::vec3 const & translation);
    glm::mat4 RotationToMatrix(glm::vec3 const & eulerAngles);
    glm::mat4 ScaleToMatrix(glm::vec3 const & scale);

    // Returns a world-space quaternion that orients an object so its
    // forward axis aligns with `direction`.
    [[nodiscard]] inline glm::quat quatFromDirection(
        const glm::vec3& direction,
        const glm::vec3& up = UpVec3)
    {
        // glm::lookAt() produces a VIEW matrix (world→camera)
        // conjugate (invert) it to get the equivalent object to world rotation
        return glm::conjugate(glm::quat_cast(glm::lookAt(glm::vec3(0.f), direction, up)));
    }

} // namespace math::transform

#pragma once

#include <glm/glm.hpp>
#include <string>

class BaseCamera {
public:
    virtual ~BaseCamera() = default;

    [[nodiscard]]
    virtual glm::mat4 viewMatrix() = 0;

    [[nodiscard]]
    virtual glm::vec3 position() { return mPosition; }

    // From the view matrix
    glm::vec3 forward();
    glm::vec3 right();
    glm::vec3 up() { return mUp; }

    virtual void adjustFov(float deltaFov);
    float fov() const { return mFov; }
    void fov(float fov);

    virtual std::string toString() const;

protected:
    BaseCamera();
    explicit BaseCamera(float fov);

    float mFov = glm::radians(120.0f);  // FOV stored in radians

    glm::vec3 mUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 1.0f);

    bool mIsDirty = true;
};

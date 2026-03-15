#pragma once

#include <glm/glm.hpp>
#include <string>

class BaseCamera {
public:
    virtual ~BaseCamera() = default;

    [[nodiscard]]
    virtual glm::mat4 getViewMatrix() = 0;

    [[nodiscard]]
    virtual glm::vec3 getPosition() = 0;

    virtual void adjustFOV(float deltaFOV);
    virtual void adjustRadius(float deltaRadius);

    float getFOV() const { return _fov; }

    virtual std::string toString() const;

protected:
    BaseCamera();
    explicit BaseCamera(float fov);

    void setFOV(float fov);

    float _fov = glm::radians(120.0f);  // FOV stored in radians

    glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 _position = glm::vec3(0.0f, 0.0f, 1.0f);

    bool _isDirty = true;
};

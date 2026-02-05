#pragma once

#include "CameraStats.hpp"
#include <glm/glm.hpp>

class BaseCamera {
public:
    virtual ~BaseCamera() = default;

    [[nodiscard]]
    virtual glm::mat4 getViewMatrix() = 0;

    [[nodiscard]]
    virtual glm::vec3 getPosition() = 0;

    virtual void adjustFOV(float deltaFOV);
    virtual void adjustScale(float deltaScale);
    virtual void adjustRadius(float deltaRadius);

    float getFOV() const { return _fov; }
    float getScale() const { return _scale; }
    float getRadius() const { return _radius; }

    virtual CameraStats getStats() = 0;

protected:
    BaseCamera();
    explicit BaseCamera(float fov, float scale);

    void setFOV(float fov);
    void setScale(float scale);
    void setRadius(float radius);

    float _fov = glm::radians(120.0f);  // FOV stored in radians
    float _scale = 1.0f;
    float _radius = 3.0f;

    float _theta = 0.0f;
    float _phi = 0.0f;

    glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 _target = glm::vec3(0.0f);
    glm::vec3 _position = glm::vec3(0.0f, 0.0f, 1.0f);

    bool _isDirty = true;
};

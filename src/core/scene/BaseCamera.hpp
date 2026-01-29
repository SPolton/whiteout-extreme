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

    float getFOV() const { return _fov; }
    float getScale() const { return _scale; }

    virtual CameraStats getStats() = 0;

protected:
    BaseCamera();
    explicit BaseCamera(float fov, float scale);

    void setFOV(float fov);
    void setScale(float scale);

    float _fov = 120.0f;
    float _scale = 1.0f;

    bool _isDirty = true;

    static float clampValue(float value, float min, float max);
};

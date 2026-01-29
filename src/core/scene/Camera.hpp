#pragma once

#include "BaseCamera.hpp"
#include "CameraStats.hpp"
#include "utils/logger.h"

class Camera : public BaseCamera {
public:
    bool isCamera2D = false;

    Camera();
    Camera(glm::vec3 up_vec);

    void adjustRadius(float amount);

    void moveVertically(float speed);
    void moveHorizontally(float speed);

    [[nodiscard]]
    glm::mat4 getViewMatrix() override;

    [[nodiscard]]
    glm::vec3 getPosition() override { return _position; }

    [[nodiscard]]
    glm::vec3 getCamPos() { return _position; }

    CameraStats getStats() override;

private:
glm::vec3 camFront;

void prepareTurnTable();
protected:
    float bound(float value, float min, float max);
};

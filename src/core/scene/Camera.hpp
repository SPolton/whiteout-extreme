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
    glm::vec3 getPosition() override { return camPos; }

    [[nodiscard]]
    glm::vec3 getCamPos() { return camPos; }

    CameraStats getStats() override;

private:
    glm::vec3 camPos, camFront, camUp, target;
    float radius, verticalAngle, horizontalAngle;

    void prepareTurnTable();
protected:
    float bound(float value, float min, float max);
};

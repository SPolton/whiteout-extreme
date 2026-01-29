#pragma once

#include "CameraStats.hpp"
#include "utils/logger.h"

class Camera {
public:
    bool isCamera2D = false;

    Camera();
    Camera(glm::vec3 up_vec);

    void adjustFOV(float amount);
    void adjustScale(float amount);
    void adjustRadius(float amount);

    void moveVertically(float speed);
    void moveHorizontally(float speed);

    glm::mat4 getViewMatrix();

    float getFOV() { return fov; }
    float getScale() { return scale; }
    glm::vec3 getCamPos() { return camPos; }

    CameraStats getStats();

private:
    glm::vec3 camPos, camFront, camUp, target;
    float radius, verticalAngle, horizontalAngle;
    float fov, scale;

    void prepareTurnTable();
protected:
    float bound(float value, float min, float max);
};

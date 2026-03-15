#pragma once

#include "BaseCamera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Free-flying FPS-style camera with WASD movement and mouse look
class FreeCamera : public BaseCamera {
public:
    enum class Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    struct Params {
        glm::vec3 initialPosition = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 initialWorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        float initialYaw = -90.0f;    // In degrees - will be converted to radians internally
        float initialPitch = 0.0f;    // In degrees - will be converted to radians internally
        float movementSpeed = 5.0f;
        float mouseSensitivity = 0.1f;
        bool constrainPitch = true;   // Prevent camera from flipping upside down
    };

    explicit FreeCamera();
    explicit FreeCamera(Params const& params);

    // Camera movement (call these in your input processing)
    void processKeyboard(Movement direction, float deltaTime);
    void processMouseMovement(float xOffset, float yOffset);

    // BaseCamera interface
    [[nodiscard]]
    glm::mat4 getViewMatrix() override;

    void adjustRadius(float /*deltaRadius*/) override {};  // Not used for FreeCamera, but required by interface

    std::string toString() const override;

    // Getters for camera attributes
    [[nodiscard]] float getYaw() const { return mYaw; }
    [[nodiscard]] float getPitch() const { return mPitch; }
    [[nodiscard]] float getMovementSpeed() const { return mMovementSpeed; }
    [[nodiscard]] float getMouseSensitivity() const { return mMouseSensitivity; }
    
    // Setters for camera attributes
    void setMovementSpeed(float speed) { mMovementSpeed = speed; }
    void setMouseSensitivity(float sensitivity) { mMouseSensitivity = sensitivity; }

private:
    // Camera attributes
    glm::vec3 mFront;
    glm::vec3 mRight;
    glm::vec3 mWorldUp;

    // Euler angles
    float mYaw;
    float mPitch;

    // Camera options
    float mMovementSpeed;
    float mMouseSensitivity;
    bool mConstrainPitch;

    // Helper function to recalculate camera vectors from updated Euler angles
    void updateCameraVectors();
};

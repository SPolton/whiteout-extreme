//
// Name: Scott Polton
// UCID: 30138102
// Date: 09/04/2025
//
// Copied from assignment 2 and modified

#pragma once

#include "core/assets/AssetPath.hpp"
#include "utils/logger.h"

// Transform class that contains information for each 3D object.
// This includes local position, uniform scale, and local rotation.
class SceneTransform
{
public:
    bool DEBUG = false;

    SceneTransform();
    SceneTransform(float scale);
    virtual ~SceneTransform() = default;

    void setParent(std::shared_ptr<SceneTransform> parent) { this->parent = parent; }
    bool hasParent() { return parent != nullptr; }

    void setPosition(glm::vec3 position);
    glm::vec3 getPosition() { return position; }
    glm::vec3 getWorldPosition();

    void setScale(float uniform);
    void adjustScale(float uniform);
    float getRadius() { return radius; }

    void setRotation(float radians);
    void adjustRotation(float radians);
    void setRotationAxis(glm::vec3 axis);

    glm::mat4 getCompleteMatrix();

    bool isOffScreen();

protected:
    glm::vec3 position;
    glm::vec3 rotationAxis;
    float uniformScale, radius;
    float angle; // Local rotation angle in radians between 0 and 2*pi

    // Update the complete matrix
    virtual void updated() { isUpdated = true; }

    // The local transforms applied to self (not including parent M)
    virtual glm::mat4 getLocalMatrixBase() { return translation * rotation * scale; };

    // The local transforms that will be inherited by children (not including parent M)
    virtual glm::mat4 getChildMatrixBase() { return getLocalMatrixBase(); };

    glm::mat4 getScale() { return scale; }
    glm::mat4 getRotation() { return rotation; }
    glm::mat4 getTranslation() { return translation; }

    float bound(float value, float min, float max);
    float normalizeAngle(float radians);

private:
    bool isUpdated = true;

    glm::mat4 scale = glm::mat4(1.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::mat4 translation = glm::mat4(1.0f);
    glm::mat4 A, B; // A is the full local matrix, B is inherited by children

    std::shared_ptr<SceneTransform> parent;
    glm::mat4 savedParentMatrix;
    glm::mat4 getParentMatrix();
};


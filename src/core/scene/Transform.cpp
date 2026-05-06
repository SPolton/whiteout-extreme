#include "Transform.hpp"
#include <ext/matrix_transform.hpp>

// Initialize with reasonable defaults
SceneTransform::SceneTransform()
{
    setScale(1);
    setRotationAxis({0, 1, 0});   // Rotate by the y-axis
    setRotation(0);
    setPosition(glm::vec3(0.0f)); // Origin in local space
}

SceneTransform::SceneTransform(float scale) : SceneTransform() {
    setScale(scale);
}

// Override the uniform scale of the object
void SceneTransform::setScale(float uniform)
{
    updated();
    uniformScale = bound(uniform, 0, uniform);
    scale = glm::scale(glm::mat4(1.0f), glm::vec3(uniformScale));
    radius = uniform;
}

// Add to the current uniform scale of the object
void SceneTransform::adjustScale(float uniform) {
    setScale(uniformScale + uniform);
}

// Override the local rotation of the object.
// Positive = counterclockwise, Negative = clockwise
void SceneTransform::setRotation(float radians)
{
    updated();
    angle = normalizeAngle(radians);
    rotation = glm::rotate(glm::mat4(1.0f), angle, rotationAxis);
}

// Add to the current local rotation of the object.
// Positive = counterclockwise, Negative = clockwise
void SceneTransform::adjustRotation(float radians) {
    setRotation(angle + radians);
}

// Set the axis of the local rotation
void SceneTransform::setRotationAxis(glm::vec3 axis)
{
    rotationAxis = axis;
    updated();
}

// Override the local position of the object
void SceneTransform::setPosition(glm::vec3 newPosition)
{
    updated();
    this->position = newPosition;
    // if (DEBUG) logger::debug("Position: ({}, {}, {})", newPosition.x, newPosition.y, newPosition.z);

    translation = glm::translate(glm::mat4(1.0f), newPosition);
}

// Returns the world space position of this transform
glm::vec3 SceneTransform::getWorldPosition()
{
    if (parent) {
        // Using parent transform to get to world space, discard the w.
        return glm::vec3(getParentMatrix() * glm::vec4(position, 1.0f));
    } else
        return position;
}

// Returns the parent matrix, or the identity matrix if no parent is set.
glm::mat4 SceneTransform::getParentMatrix() {
    if (parent)
        return parent->B; // The one with elements we want to inherit
    else
        return glm::mat4(1.0f);
}

// Returns the combined matrix of Scale -> Roation -> Translation
glm::mat4 SceneTransform::getCompleteMatrix()
{
    glm::mat4 parentM = getParentMatrix();
    if (isUpdated || savedParentMatrix != parentM) {
        isUpdated = false;
        savedParentMatrix = parentM;

        // Apply all the transforms
        A = parentM * getLocalMatrixBase(); // translation * rotation * scale;
        B = parentM * getChildMatrixBase();
        // if (DEBUG) logger::debug("A = {}", glm::to_string(A));
    }
    return A;
}

// Returns true if the whole sphere radius is outside the screen
bool SceneTransform::isOffScreen()
{
    return false;
}

// Helper function to bound a value between a min and max
// Made in the first assignment and copied here
float SceneTransform::bound(float value, float min, float max)
{
	if (value < min)
		return min;
	else if (value > max)
		return max;
	else
		return value;
}

// Helper to normalize the angle to between 0 and 2*pi radians
// Not strictly necessary
float SceneTransform::normalizeAngle(float radians)
{
    radians = fmod(radians, glm::two_pi<float>());
    if (radians < 0)
        radians += glm::two_pi<float>(); // Negative to positive
    return radians;
}

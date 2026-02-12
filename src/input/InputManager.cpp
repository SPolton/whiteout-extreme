#include "InputManager.hpp"

#include "utils/logger.h"
#include <utility>

InputManager::InputManager(
    ResizeCallback resizeCallback,
    MouseWheelCallback mouseWheelCallback
)
    : mResizeCallback(std::move(resizeCallback))
    , mMouseWheelCallback(std::move(mouseWheelCallback))
{
}

void InputManager::keyCallback(
    int const key,
    int const /*scancode*/,
    int const action,
    int const /*mods*/
)
{
    // logger::info("KeyCallback: key={}, action={}", key, action);
    if (action == GLFW_PRESS)
    {
        mKeyStatusMap[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        mKeyStatusMap[key] = false;
        mKeyConsumedMap[key] = false;
    }
}

void InputManager::windowSizeCallback(int const width, int const height)
{
    logger::info("WindowSizeCallback: width={}, height={}", width, height);
    if (mResizeCallback != nullptr)
    {
        mResizeCallback(width, height);
    }
}

void InputManager::mouseButtonCallback(int const button, int const action, int /*mods*/)
{
    // logger::info("MouseButtonCallback: button={}, action={}", button, action);
    mMouseStatusMap[button] = action;
    if (action == GLFW_RELEASE)
    {
        mMouseConsumedMap[button] = false;
    }
}

void InputManager::cursorPosCallback(double const xpos, double const ypos)
{
    // logger::debug("CursorPosCallback: xpos={}, ypos={}", xpos, ypos);
    mCursorPosition.x = xpos;
    mCursorPosition.y = ypos;
}

void InputManager::scrollCallback(double const xoffset, double const yoffset)
{
    mMouseWheelCallback(xoffset, yoffset);
}

[[nodiscard]]
bool InputManager::isKeyPressed(int const keyboardButton) const
{
    bool isButtonDown = false;
    auto const findResult = mKeyStatusMap.find(keyboardButton);
    if (findResult != mKeyStatusMap.end())
    {
        isButtonDown = findResult->second;
    }
    return isButtonDown;
}

bool InputManager::isKeyPressedOnce(int const keyboardButton)
{
    auto const findResult = mKeyStatusMap.find(keyboardButton);
    if (findResult != mKeyStatusMap.end() && findResult->second)
    {
        auto const consumedResult = mKeyConsumedMap.find(keyboardButton);
        if (consumedResult == mKeyConsumedMap.end() || !consumedResult->second)
        {
            mKeyConsumedMap[keyboardButton] = true;
            logger::info("Key {} pressed once.", keyboardButton);
            return true;
        }
    }
    return false;
}

bool InputManager::isMousePressed(int const mouseButton) const
{
    bool isButtonDown = false;
    auto const findResult = mMouseStatusMap.find(mouseButton);
    if (findResult != mMouseStatusMap.end())
    {
        isButtonDown = findResult->second;
    }
    return isButtonDown;
}

bool InputManager::isMousePressedOnce(int const mouseButton)
{
    auto const findResult = mMouseStatusMap.find(mouseButton);
    if (findResult != mMouseStatusMap.end() && findResult->second)
    {
        auto const consumedResult = mMouseConsumedMap.find(mouseButton);
        if (consumedResult == mMouseConsumedMap.end() || !consumedResult->second)
        {
            mMouseConsumedMap[mouseButton] = true;
            logger::info("Mouse button {} pressed once.", mouseButton);
            return true;
        }
    }
    return false;
}

glm::dvec2 const &InputManager::cursorPosition() const
{
    return mCursorPosition;
}

// Controller Input Handling
//==================================================================================================================//

void InputManager::pollControllerInputs() {
    // use gamepad mapping (need to test)
    // we are assuming only one controller right now: GLFW_JOYSTICK_1

    // check if joystick is present AND has a gamepad mapping
    // use gamepad functions for standard gamepads like Xbox controllers
    if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1))
    {
        // if successful, means we are connected. Update connection state
        if (!controllerConnected) {
            controllerConnected = true;
        
            // reteives name of the gamepad mapping
            const char* name = glfwGetGamepadName(GLFW_JOYSTICK_1);
            logger::info("Controller {} connected.", name);
        }

        // retreive gamepad state
        GLFWgamepadstate state;

        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
        {
            // if state retreived, save all info to the maps
            // Each element in the state.button array is either GLFW_PRESS or GLFW_RELEASE
            // Each element in the state.axes array is a value between -1.0 and 1.0

            // GLFW_GAMEPAD_BUTTON_LAST is a constant equal to the largest available index in the button array
            // go through each button index and store its state in our array
            for (int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST + 1; i++) {
                controllerButtons[i] = state.buttons[i];
                if (state.buttons[i] == GLFW_RELEASE) {
                    controllerButtonConsumed[i] = false;
                }
            }

            // GLFW_GAMEPAD_AXIS_LAST is a constant equal to the largest available index in the axis array
            // go through each axis index and store its state in our array
            for (int j = 0; j < GLFW_GAMEPAD_AXIS_LAST + 1; j++) {
                controllerAxes[j] = state.axes[j];
            }
        }
    }
    else {
        // controller has disconnected, clear all controller state info
        if (controllerConnected) {
            logger::info("Controller disconnected.");
            controllerConnected = false;
            controllerButtons.clear();
            controllerButtonConsumed.clear();
            controllerAxes.clear();
        }
    }
}

bool InputManager::IsControllerButtonDown(int const controllerButton) const
{
    // initialize to false
    bool isButtonDown = false;

    // only if it is pressed, return true
    if (controllerButtons.at(controllerButton) == GLFW_PRESS) {
        return true;
    };

    return isButtonDown;
}

float InputManager::GetControllerAxis(int const controllerAxis) const
{
    auto const findResult = controllerAxes.find(controllerAxis);
    if (findResult != controllerAxes.end())
    {
        return findResult->second;
    }
    return 0.0f;
}

bool InputManager::IsControllerConnected() {
    return controllerConnected;
}

bool InputManager::isControllerButtonPressedOnce(int const controllerButton)
{
    auto const findResult = controllerButtons.find(controllerButton);
    if (findResult != controllerButtons.end() && findResult->second == GLFW_PRESS)
    {
        auto const consumedResult = controllerButtonConsumed.find(controllerButton);
        if (consumedResult == controllerButtonConsumed.end() || !consumedResult->second)
        {
            controllerButtonConsumed[controllerButton] = true;
            logger::info("Controller button {} pressed once.", controllerButton);
            return true;
        }
    }
    return false;
}

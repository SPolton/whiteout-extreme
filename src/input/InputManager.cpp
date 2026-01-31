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
    int const scancode,
    int const action,
    int const mods
)
{
    logger::info("KeyCallback: key={}, action={}", key, action);
    if (action == GLFW_PRESS)
    {
        mKeyStatusMap[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        mKeyStatusMap[key] = false;
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

void InputManager::mouseButtonCallback(int const button, int const action, int mods)
{
    logger::info("MouseButtonCallback: button={}, action={}", button, action);
    mMouseStatusMap[button] = action;
}

void InputManager::cursorPosCallback(double const xpos, double const ypos)
{
    logger::debug("CursorPosCallback: xpos={}, ypos={}", xpos, ypos);
    mCursorPosition.x = xpos;
    mCursorPosition.y = ypos;
}

void InputManager::scrollCallback(double const xoffset, double const yoffset)
{
    mMouseWheelCallback(xoffset, yoffset);
}

[[nodiscard]]
bool InputManager::IsKeyboardButtonDown(int const keyboardButton) const
{
    bool isButtonDown = false;
    auto const findResult = mKeyStatusMap.find(keyboardButton);
    if (findResult != mKeyStatusMap.end())
    {
        isButtonDown = findResult->second;
    }
    return isButtonDown;
}

bool InputManager::IsMouseButtonDown(int const mouseButton) const
{
    bool isButtonDown = false;
    auto const findResult = mMouseStatusMap.find(mouseButton);
    if (findResult != mMouseStatusMap.end())
    {
        isButtonDown = findResult->second;
    }
    return isButtonDown;
}

glm::dvec2 const &InputManager::CursorPosition() const
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
        controllerConnected = true;
        
        // reteives name of the gamepad mapping
        const char* name = glfwGetGamepadName(GLFW_JOYSTICK_1);

        // retreive gamepad state
        GLFWgamepadstate state;

        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
        {
            // if state retreived, save all info to the maps
            // Each element in the state.button array is either GLFW_PRESS or GLFW_RELEASE
            // Each element in the state.axes array is a value between -1.0 and 1.0

            // GLFW_GAMEPAD_BUTTON_LAST is a constant equal to the largest available index in the button array
            // go through each button index and store its state in our array
            for (int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST; ++i) {
                controllerButtons[i] = state.buttons[i];
            }

            // GLFW_GAMEPAD_AXIS_LAST is a constant equal to the largest available index in the axis array
            // go through each axis index and store its state in our array
            for (int j = 0; j < GLFW_GAMEPAD_AXIS_LAST; ++j) {
                controllerAxes[j] = state.axes[j];
            }
        }

        // using pure joystick inputs
        /*
        // get positions of all axes of a joystick
        int countAxes;
        // Each element in the returned array is a value between -1.0 and 1.0
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &countAxes);
        // store the retreived info to the maps
        for (int i = 0; i < countAxes; i++) {
            controllerAxes[i] = axes[i];
        }

        // get all button states
        int countButtons;
        // Each element in the returned array is either GLFW_PRESS or GLFW_RELEASE
        const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &countButtons);
        // store the retreived info to the maps
        for (int i = 0; i < countButtons; i++) {
            // if it is GLFW_PRESS, then store true, otherwise store false to represent GLFW_RELEASE
            if (buttons[i] == buttons[i]) {
                controllerButtons[i] = true;
            }
            else {
                controllerButtons[i] = false;
            }
        }
        */
    }
    else {
        // controller has disconnected, clear all controller state info
        controllerConnected = false;
        controllerButtons.clear();
        controllerAxes.clear();
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
    // initialize to 0
    bool axisValue = 0;

    axisValue = controllerAxes.at(controllerAxis);

    return axisValue;
}

bool InputManager::IsControllerConnected() {
    return controllerConnected;
}

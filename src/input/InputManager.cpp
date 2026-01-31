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
    logger::info("KeyCallback: key={}, scancode={}, action={}", key, action);
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

void InputManager::mouseButtonCallback(int const button, int const action, int /*mods*/)
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

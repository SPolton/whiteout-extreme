#pragma once

#include "Window.hpp"

#include <functional>
#include <unordered_map>

class InputManager final : public CallbackInterface
{
public:

    using ResizeCallback = std::function<void(int width, int height)>;

    using MouseWheelCallback = std::function<void(double xOffset, double yOffset)>;

    explicit InputManager(
        ResizeCallback resizeCallback,
        MouseWheelCallback mouseWheelCallback
    );

    virtual ~InputManager() = default;

    [[nodiscard]]
    bool isKeyPressed(int keyboardButton) const;

    [[nodiscard]]
    bool isKeyPressedOnce(int keyboardButton);

    [[nodiscard]]
    bool isMousePressed(int mouseButton) const;

    [[nodiscard]]
    bool isMousePressedOnce(int mouseButton);

    [[nodiscard]]
    glm::dvec2 const & cursorPosition() const;

    // controller queries
    void pollControllerInputs();

    bool IsControllerButtonDown(int controllerButton) const;

    float GetControllerAxis(int controllerAxis) const;

    bool IsControllerConnected();

    bool isControllerButtonPressedOnce(int const controllerButton);

private:

    void keyCallback(
        int key,
        int scancode,
        int action,
        int mods
    ) override;

    void windowSizeCallback(int width, int height) override;

    void mouseButtonCallback(int button, int action, int mods) override;

    void cursorPosCallback(double xpos, double ypos) override;

    void scrollCallback(double xoffset, double yoffset) override;

    // keyboard input
    std::unordered_map<int, bool> mKeyStatusMap{};
    std::unordered_map<int, bool> mKeyConsumedMap{};

    // mouse input
    std::unordered_map<int, bool> mMouseStatusMap{};
    std::unordered_map<int, bool> mMouseConsumedMap{};
    glm::dvec2 mCursorPosition{};
    ResizeCallback mResizeCallback;
    MouseWheelCallback mMouseWheelCallback;

    // controller input
    std::unordered_map<int, bool> controllerButtons;
    std::unordered_map<int, bool> controllerButtonConsumed{};
    std::unordered_map<int, float> controllerAxes;
    bool controllerConnected = false;
};

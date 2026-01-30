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
    bool IsKeyboardButtonDown(int keyboardButton) const;

    [[nodiscard]]
    bool IsMouseButtonDown(int mouseButton) const;

    [[nodiscard]]
    glm::dvec2 const & CursorPosition() const;

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

    std::unordered_map<int, bool> mKeyStatusMap{};
    std::unordered_map<int, bool> mMouseStatusMap{};
    glm::dvec2 mCursorPosition{};
    ResizeCallback mResizeCallback;
    MouseWheelCallback mMouseWheelCallback;

};

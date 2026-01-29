#pragma once

#include <imgui.h>
#include <glm/glm.hpp>
#include "core/scene/CameraStats.hpp"

// Handles ImGui panel rendering and settings for RenderingSystem
// Separates UI logic from rendering logic
class ImGuiPanel {
public:
    ImGuiPanel();
    virtual ~ImGuiPanel() = default;

    void update();

    // GETTERS for rendering system to use
    glm::vec3 getBackgroundColor() const { return backgroundColor; }
    bool getShowWireframe() const { return showWireframe; }
    bool getShowNormals() const { return showNormals; }
    float getAnimationSpeed() const { return animationSpeed; }
    
    // Camera stats (set by rendering system)
    CameraStats cameraStats;

protected:
    // Individual panel sections
    void renderDebugInfo();
    void renderRenderSettings();
    void renderCameraInfo();
    void renderControls();

private:
    // Rendering settings
    glm::vec3 backgroundColor;
    bool showWireframe;
    bool showNormals;
    float animationSpeed;
    
    // Window settings
    bool showDebugWindow;
    bool showSettingsWindow;
};

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

    // For simplicity, expose settings directly (no getters)
    // Camera stats (set by rendering system)
    CameraStats cameraStats;

    // Rendering settings
    glm::vec3 getBackgroundColor() const { return backgroundColor; }
    bool showWireframe = false;
    bool showNormals = false;
    float animationSpeed = 1.0;
    
    // Camera control settings
    float camSpeed = 50.0f;
    float camZoomSpeed = 5.0f;

    // Window settings
    bool showDebugWindow = true;
    bool showSettingsWindow = true;

protected:
    // Individual panel sections
    void renderDebugInfo();
    void renderRenderSettings();
    void renderCameraInfo();
    void renderControls();

private:
    glm::vec3 backgroundColor;
};

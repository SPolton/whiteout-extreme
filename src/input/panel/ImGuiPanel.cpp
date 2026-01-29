#include "ImGuiPanel.hpp"

ImGuiPanel::ImGuiPanel()
    : backgroundColor(0.2f, 0.3f, 0.3f)
    , showWireframe(false)
    , showNormals(false)
    , animationSpeed(1.0f)
    , showDebugWindow(true)
    , showSettingsWindow(true)
{
}

void ImGuiPanel::update()
{
    // Main debug info window
    if (showDebugWindow) {
        renderDebugInfo();
    }

    // Settings window
    if (showSettingsWindow) {
        ImGui::Begin("Settings", &showSettingsWindow);
        renderRenderSettings();
        renderCameraInfo();
        renderControls();
        ImGui::End();
    }
}

void ImGuiPanel::renderDebugInfo()
{
    ImGui::Begin("Debug Info", &showDebugWindow);
    
    // FPS counter
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
    
    ImGui::Separator();
    
    // Renderer info
    ImGui::Text("Renderer: OpenGL");
    ImGui::Text("Triangles rendered: 1"); // Simple for now
    
    ImGui::End();
}

void ImGuiPanel::renderRenderSettings()
{
    if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::ColorEdit3("Background Color", &backgroundColor[0]);
        ImGui::Checkbox("Wireframe Mode", &showWireframe);
        ImGui::Checkbox("Show Normals", &showNormals);
        ImGui::SliderFloat("Animation Speed", &animationSpeed, 0.0f, 5.0f);
        
        if (ImGui::Button("Reset Defaults")) {
            backgroundColor = glm::vec3(0.2f, 0.3f, 0.3f);
            showWireframe = false;
            showNormals = false;
            animationSpeed = 1.0f;
        }
    }
}

void ImGuiPanel::renderCameraInfo()
{
    if (ImGui::CollapsingHeader("Camera")) {
        glm::vec3 pos = cameraStats.camPos;
        glm::vec3 target = cameraStats.target;
        
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
        ImGui::Text("Target:   (%.2f, %.2f, %.2f)", target.x, target.y, target.z);
        ImGui::Text("Radius: %.2f", cameraStats.radius);
        ImGui::Text("FOV: %.2f", cameraStats.fov);
        ImGui::Text("Scale: %.2f", cameraStats.scale);
        ImGui::Text("Aspect: %.2f", cameraStats.aspect);
    }
}

void ImGuiPanel::renderControls()
{
    if (ImGui::CollapsingHeader("Controls")) {
        ImGui::BulletText("ESC - Exit application");
        ImGui::BulletText("Right Mouse - Rotate camera");
        ImGui::BulletText("Mouse Wheel - Zoom");
    }
}

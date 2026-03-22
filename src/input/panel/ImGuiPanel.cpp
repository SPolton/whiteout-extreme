#include "ImGuiPanel.hpp"

const glm::vec3 defaultBackgroundColor(0.47f, 0.82f, 1.0f);

ImGuiPanel::ImGuiPanel()
    : backgroundColor(defaultBackgroundColor)
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
        ImGui::SetNextWindowPos(ImVec2(600, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Settings", &showSettingsWindow);
        renderRenderSettings();
        renderCameraInfo();
        renderControls();
        ImGui::End();
    }
}

void ImGuiPanel::renderDebugInfo()
{
    ImGui::SetNextWindowPos(ImVec2(10, 450), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_FirstUseEver);
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
            backgroundColor = defaultBackgroundColor;
            showWireframe = false;
            showNormals = false;
            animationSpeed = 1.0f;
        }
    }
}

void ImGuiPanel::renderCameraInfo()
{
    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::Text("%s", cameraInfo.c_str());
        ImGui::Text("Aspect: %.2f", aspectRatio);
        
        ImGui::Separator();
        ImGui::SliderFloat("Camera Speed", &camSpeed, 1.0f, 10.0f);
        ImGui::SliderFloat("Zoom Speed", &camZoomSpeed, 0.1f, 20.0f);
    }
}

void ImGuiPanel::renderControls()
{
    if (ImGui::CollapsingHeader("Controls")) {
        ImGui::BulletText("ESC - Exit application");
        ImGui::BulletText("Right Mouse + Drag - Rotate camera");
        ImGui::BulletText("Mouse Wheel - Zoom in/out");
    }
}

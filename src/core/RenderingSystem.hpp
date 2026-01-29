#pragma once

#include "input/Window.hpp"
#include "core/render/ShaderProgram.hpp"
#include "core/buffer/Geometry.hpp"
#include "core/scene/Camera.hpp"
#include "input/panel/ImGuiWrapper.hpp"
#include "input/panel/ImGuiPanel.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class RenderingSystem {
public:
    RenderingSystem();
    ~RenderingSystem();

    void update();
    void updateUI();
    void endFrame();
    bool shouldClose() const;

private:
    // Core components following modular architecture
    std::unique_ptr<Window> window;
    std::unique_ptr<ShaderProgram> shader;
    std::unique_ptr<Camera> camera;
    
    // Geometry using RAII wrappers
    std::unique_ptr<GPU_Geometry> triangleGeometry;
    std::unique_ptr<CPU_Geometry> triangleCPUData;
    
    // ImGui management (separated concerns)
    std::unique_ptr<ImGuiWrapper> imguiWrapper;  // Handles lifecycle
    std::unique_ptr<ImGuiPanel> imguiPanel;       // Handles content
    
    bool init();
    void processInput();
    void render();
    glm::mat4 getProjectionMatrix() const;
};

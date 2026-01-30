#include "ImGuiWrapper.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

bool ImGuiWrapper::init(GLFWwindow* window)
{
    if (initialized) return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = "imgui.ini";
    
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        return false;
    }
    
    if (!ImGui_ImplOpenGL3_Init("#version 330"))
    {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    initialized = true;
    return true;
}

void ImGuiWrapper::shutdown()
{
    if (!initialized) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    initialized = false;
}

void ImGuiWrapper::beginFrame()
{
    if (!initialized) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiWrapper::endFrame()
{
    if (!initialized) return;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiWrapper::renderFPS()
{
    if (!initialized) return;

    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::Begin("FPS", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoBackground
    );

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Avg %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}

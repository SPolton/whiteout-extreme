#include "ImGuiWrapper.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

bool ImGuiWrapper::init(GLFWwindow* window)
{
    if (initialized) return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
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

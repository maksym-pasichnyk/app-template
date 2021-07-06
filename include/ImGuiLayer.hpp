#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <utils/matches.hpp>
#include <Event.hpp>

struct ImGuiLayer {
    ImGuiLayer() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

        ImGui_ImplOpenGL3_Init("#version 150");
    }

    ~ImGuiLayer() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
    }

    void begin() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
    }

    void end() {
        ImGui::Render();
    }

    void flush() {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    bool handleEvent(const Event& event) {
        auto& io = ImGui::GetIO();

        return matches(event,
           [&io](const KeyEvent& e) -> bool {
               if (e.action == GLFW_PRESS) {
                   io.KeysDown[e.key] = true;
               } else if (e.action == GLFW_RELEASE) {
                   io.KeysDown[e.key] = false;
               }

               io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
               io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
               io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
               io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
               return false;
           },
           [&io](const MouseButtonEvent& e) {
               if (e.action == GLFW_PRESS) {
                   io.MouseDown[e.button] = true;
               } else if (e.action == GLFW_RELEASE) {
                   io.MouseDown[e.button] = false;
               }
               return false;
           },
           [&io](const MouseMoveEvent& e) {
               io.MousePos = ImVec2(static_cast<float>(e.xpos), static_cast<float>(e.ypos));
               return false;
           },
           [](const auto& _) -> bool { return false; }
        );
    }
};

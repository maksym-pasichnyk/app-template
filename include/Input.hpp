#pragma once

#include <array>
#include <Event.hpp>
#include <glm/vec2.hpp>
#include <GLFW/glfw3.h>
#include <utils/matches.hpp>

struct Input {
    enum class State : uint8_t {
        None,
        JustPress,
        JustRelease,
        Up,
        Down,
        Press
    };

    std::array<State, GLFW_MOUSE_BUTTON_LAST + 1> MouseDown{};
    std::array<State, GLFW_KEY_LAST + 1> KeysDown{};
    glm::ivec2 LastMousePos{};
    glm::ivec2 MousePos{};

    void handleEvent(const KeyEvent& e) {
        if (e.action == GLFW_PRESS) {
            KeysDown[e.key] = State::JustPress;
        } else if (e.action == GLFW_RELEASE) {
            KeysDown[e.key] = State::JustRelease;
        }
    }

    void handleEvent(const MouseMoveEvent& e) {
        MousePos = { static_cast<int>(e.xpos), static_cast<int>(e.ypos) };
    }

    void handleEvent(const MouseButtonEvent& e) {
        if (e.action == GLFW_PRESS) {
            MouseDown[e.button] = State::JustPress;
        } else if (e.action == GLFW_RELEASE) {
            MouseDown[e.button] = State::JustRelease;
        }
    }

    void update() {
        static constexpr std::array NextState {
            State::None,
            State::Down,
            State::Up,
            State::None,
            State::Press,
            State::Press
        };
        for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST + 1; ++i) {
            MouseDown[i] = NextState[static_cast<size_t>(MouseDown[i])];
        }
        for (int i = 0; i < GLFW_KEY_LAST + 1; ++i) {
            KeysDown[i] = NextState[static_cast<size_t>(KeysDown[i])];
        }

        LastMousePos = MousePos;
    }

    glm::ivec2 getMousePosition() const {
        return MousePos;
    }

    State getButtonState(int button) const {
        return KeysDown[button];
    }

    State getMouseButtonState(int button) const {
        return MouseDown[button];
    }

    bool isMouseButtonDown(int button) const {
        return MouseDown[button] == State::Down;
    }

    bool isMouseButtonUp(int button) const {
        return MouseDown[button] == State::Up;
    }

    bool isMouseButton(int button) const {
        return MouseDown[button] == State::Down || MouseDown[button] == State::Press;
    }

    bool isButtonDown(int button) const {
        return KeysDown[button] == State::Down;
    }

    bool isButtonUp(int button) const {
        return KeysDown[button] == State::Up;
    }

    bool isButton(int button) const {
        return KeysDown[button] == State::Down || KeysDown[button] == State::Press;
    }
};
#pragma once

#include <variant>

struct WindowResizeEvent {
    int width;
    int height;
};

struct FramebufferResizeEvent {
    int width;
    int height;
};

struct WindowCloseEvent {};

struct KeyEvent {
    int key;
    int scancode;
    int action;
    int mods;
};

struct MouseButtonEvent {
    int button;
    int action;
    int mods;
};

struct MouseMoveEvent {
    double xpos;
    double ypos;
};

struct FocusEvent {
    bool focused;
};

struct QuitEvent {};

using Event = std::variant<
    WindowResizeEvent,
    FramebufferResizeEvent,
    WindowCloseEvent,
    KeyEvent,
    MouseMoveEvent,
    MouseButtonEvent,
    FocusEvent,
    QuitEvent
>;
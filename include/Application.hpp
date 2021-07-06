#pragma once

#include <chrono>
#include <memory>
#include <concepts>

#include <Event.hpp>
#include <Window.hpp>
#include <RenderContext.hpp>

template <typename T>
concept HasHandleEvent = requires(T& self, const Event& e) {
    { self.handleEvent(e) } -> std::same_as<void>;
};

template <typename T>
concept HasUpdate = requires(T& self, double dt) {
    { self.update(dt) } -> std::same_as<void>;
};

template <typename T>
concept HasRenderFrame = requires(T& self) {
    { self.renderFrame() } -> std::same_as<void>;
};

template <typename T>
struct Application {
    std::unique_ptr<Window> window;
    std::unique_ptr<RenderContext> renderContext;

    Application(const char* title, int width, int height) {
        window = std::make_unique<Window>(title, width, height);
        renderContext = std::make_unique<RenderContext>();
    }

    void handleEvents() {
        window->pumpEvents();

        while (auto event = window->pollEvent()) {
            if constexpr (HasHandleEvent<T>) {
                static_cast<T&>(*this).handleEvent(*event);
            }
        }
    }

    void run() {
        using Clock = std::chrono::high_resolution_clock;

        auto last_time = Clock::now();
        while (!window->shouldClose()) {
            const auto current_time = Clock::now();
            const auto delta_time = current_time - std::exchange(last_time, current_time);

            handleEvents();

            if constexpr (HasUpdate<T>) {
                static_cast<T&>(*this).update(std::chrono::duration<double>(delta_time).count());
            }

            if constexpr (HasRenderFrame<T>) {
                static_cast<T&>(*this).renderFrame();
            }

            window->swapBuffers();
        }
    }
};
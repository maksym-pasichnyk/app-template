#include <variant>
#include <cmath>
#include <array>
#include <fstream>
#include <bitset>
#include <fmt/format.h>
#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <Camera.hpp>
#include <Event.hpp>
#include <chrono>
#include <stack>

#include <Window.hpp>

template <typename... Tp, typename... Fs>
decltype(auto) matches(const std::variant<Tp...>& val, Fs&&... fs) {
    struct overloaded : Fs... { using Fs::operator()...; };
    return std::visit(overloaded { std::forward<Fs>(fs)... }, val);
}

struct Transform {
    glm::vec2 rotation{};
    glm::vec3 position{};

    auto getRotationMatrix() const -> glm::mat4 {
        const auto ry = glm::radians(rotation.x);
        const auto rp = glm::radians(rotation.y);

        const auto [sy, cy] = std::pair{ glm::sin(ry), glm::cos(ry) };
        const auto [sp, cp] = std::pair{ glm::sin(rp), glm::cos(rp) };

        return {
            cy, sp * sy, -cp * sy, 0,
            0, cp, sp, 0,
            sy, -sp * cy, cp * cy, 0,
            0, 0, 0, 1
        };
    }

    auto getTransformMatrix() const -> glm::mat4 {
        return glm::translate(getRotationMatrix(), -position);
    }

    auto getTransformMatrix(glm::vec3 offset) const -> glm::mat4 {
        return glm::translate(getRotationMatrix(), -(position + offset));
    }

    auto up() const -> glm::vec3 {
        return glm::vec3(0, 1, 0) * glm::mat3(getRotationMatrix());
    }

    auto forward() const -> glm::vec3 {
        return glm::vec3(0, 0, 1) * glm::mat3(getRotationMatrix());
    }

    auto right() const -> glm::vec3 {
        return glm::vec3(1, 0, 0) * glm::mat3(getRotationMatrix());
    }
};

struct AppPlatform {
    static std::optional<std::string> readFile(const std::string& path) {
        if (std::ifstream file{path, std::ios::in}) {
            std::stringstream stream{};
            stream << file.rdbuf();
            file.close();
            return stream.str();
        }
        return std::nullopt;
    }
};

struct CameraConstants {
    glm::mat4 transform;
    glm::vec4 position;
};

struct RenderFrame {
    GLuint framebuffer;
    GLuint color_attachment;
    GLuint depth_attachment;

    GLuint camera;
    void* constants;
};

struct RenderContext {
    RenderContext() {
        gl3wInit();

        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debug, nullptr);
    }

    static GLuint compileShader(const std::string& source, GLenum type) {
        auto data = source.data();
        auto size = GLint(source.size());

        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &data, &size);
        glCompileShader(shader);

        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        if (length > 0) {
            std::basic_string<char> infoLog{};
            infoLog.resize(length);
            glGetShaderInfoLog(shader, length, &length, infoLog.data());
            fmt::print("{}\n", infoLog);
        }

        GLint status = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    static GLuint createShader(const std::string& vertex_path, const std::string& fragment_path) {
        auto vertex = compileShader(AppPlatform::readFile(vertex_path).value(), GL_VERTEX_SHADER);
        auto fragment = compileShader(AppPlatform::readFile(fragment_path).value(), GL_FRAGMENT_SHADER);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);

        glLinkProgram(program);

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        GLint length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        if (length > 0) {
            std::basic_string<char> infoLog{};
            infoLog.resize(length);
            glGetProgramInfoLog(program, length, &length, &infoLog[0]);
            fmt::print("{}\n", infoLog);
        }

        GLint status = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            glDeleteProgram(program);
            return 0;
        }
        return program;
    }

private:
    static void debug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param) {
        const auto source_str = [source]() -> std::string_view {
            switch (source) {
                case GL_DEBUG_SOURCE_API:
                    return "API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                    return "WINDOW SYSTEM";
                case GL_DEBUG_SOURCE_SHADER_COMPILER:
                    return "SHADER COMPILER";
                case GL_DEBUG_SOURCE_THIRD_PARTY:
                    return "THIRD PARTY";
                case GL_DEBUG_SOURCE_APPLICATION:
                    return "APPLICATION";
                case GL_DEBUG_SOURCE_OTHER:
                    return "OTHER";
                default:
                    return "UNKNOWN";
            }
        }();

        const auto type_str = [type]() -> std::string_view {
            switch (type) {
                case GL_DEBUG_TYPE_ERROR:
                    return "ERROR";
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                    return "DEPRECATED_BEHAVIOR";
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                    return "UNDEFINED_BEHAVIOR";
                case GL_DEBUG_TYPE_PORTABILITY:
                    return "PORTABILITY";
                case GL_DEBUG_TYPE_PERFORMANCE:
                    return "PERFORMANCE";
                case GL_DEBUG_TYPE_MARKER:
                    return "MARKER";
                case GL_DEBUG_TYPE_OTHER:
                    return "OTHER";
                default:
                    return "UNKNOWN";
            }
        }();

        const auto severity_str = [severity]() -> std::string_view {
            switch (severity) {
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                    return "NOTIFICATION";
                case GL_DEBUG_SEVERITY_LOW:
                    return "LOW";
                case GL_DEBUG_SEVERITY_MEDIUM:
                    return "MEDIUM";
                case GL_DEBUG_SEVERITY_HIGH:
                    return "HIGH";
                default:
                    return "UNKNOWN";
            }
        }();

        fmt::print("{}, {}, {}, {}: {}\n", source_str, type_str, severity_str, id, message);
    }
};

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

struct Application {
    std::unique_ptr<Window> window;
    std::unique_ptr<RenderContext> renderContext;

    glm::ivec2 displaySize;
    Viewport viewport{};

    int frameIndex = 0;
    std::array<RenderFrame, 2> frames{};

    Transform transform{};

    std::unique_ptr<Camera> camera;
    std::unique_ptr<ImGuiLayer> imgui;

    Application(const char* title, int width, int height)
        : displaySize(width, height) {

        window = std::make_unique<Window>(title, width, height);
        renderContext = std::make_unique<RenderContext>();

        createFrames(width, height);

        imgui = std::make_unique<ImGuiLayer>();
        camera = std::make_unique<Camera>();
        camera->setSize(width, height);

        const auto framebufferSize = window->getFramebufferSize();
        viewport = {0, 0, framebufferSize.x, framebufferSize.y};
    }

    void createFrames(int width, int height) {
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

        for (int i = 0; i < 2; i++) {
            glCreateBuffers(1, &frames[i].camera);
            glNamedBufferStorage(frames[i].camera, sizeof(CameraConstants), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
            frames[i].constants = glMapNamedBufferRange(frames[i].camera, 0, sizeof(CameraConstants), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

            glCreateTextures(GL_TEXTURE_2D, 1, &frames[i].color_attachment);
            glTextureStorage2D(frames[i].color_attachment, 1, GL_RGB8, width, height);
            glTextureParameteri(frames[i].color_attachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(frames[i].color_attachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glCreateRenderbuffers(1, &frames[i].depth_attachment);
            glNamedRenderbufferStorage(frames[i].depth_attachment, GL_DEPTH32F_STENCIL8, width, height);

            glCreateFramebuffers(1, &frames[i].framebuffer);
            glNamedFramebufferTexture(frames[i].framebuffer, GL_COLOR_ATTACHMENT0, frames[i].color_attachment, 0);
            glNamedFramebufferRenderbuffer(frames[i].framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, frames[i].depth_attachment);

            if (glCheckNamedFramebufferStatus(frames[i].framebuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                fmt::print("Framebuffer is not complete!: {}\n", glGetError());
            }
        }
    }

    void flipFrame() {
        window->swapBuffers();
        frameIndex = (frameIndex + 1) % 2;
    }

    void setupCamera() {
        const auto projection_matrix = camera->getProjection();
        const auto transform_matrix = transform.getTransformMatrix();
        const auto camera_matrix = projection_matrix * transform_matrix;

        CameraConstants camera_constants {
            .transform = camera_matrix,
        };

        std::memcpy(frames[frameIndex].constants, &camera_constants, sizeof(CameraConstants));
    }

    void handleEvents() {
        window->pumpEvents();

        while (auto event = window->pollEvent()) {
            if (imgui->handleEvent(*event)) {
                continue;
            }

            matches(*event,
                [this](const WindowResizeEvent& e) {
                    displaySize = {e.width, e.height};

                    for (int i = 0; i < 2; i++) {
                        glDeleteTextures(1, &frames[i].color_attachment);
                        glCreateTextures(GL_TEXTURE_2D, 1, &frames[i].color_attachment);
                        glTextureStorage2D(frames[i].color_attachment, 1, GL_RGB8, e.width, e.height);
                        glTextureParameteri(frames[i].color_attachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTextureParameteri(frames[i].color_attachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                        glDeleteRenderbuffers(1, &frames[i].depth_attachment);
                        glCreateRenderbuffers(1, &frames[i].depth_attachment);
                        glNamedRenderbufferStorage(frames[i].depth_attachment, GL_DEPTH32F_STENCIL8, e.width, e.height);

                        glDeleteFramebuffers(1, &frames[i].framebuffer);
                        glCreateFramebuffers(1, &frames[i].framebuffer);
                        glNamedFramebufferTexture(frames[i].framebuffer, GL_COLOR_ATTACHMENT0, frames[i].color_attachment, 0);
                        glNamedFramebufferRenderbuffer(frames[i].framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, frames[i].depth_attachment);

                        if (glCheckNamedFramebufferStatus(frames[i].framebuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                            fmt::print("Framebuffer is not complete!: {}\n", glGetError());
                        }
                    }
                    return true;
                },
                [this](const FramebufferResizeEvent& e) {
                    viewport = {0, 0, e.width, e.height};
                    return true;
                },
                [](const auto& _) -> bool { return true; }
            );
        }
    }

    void beginFrame(const glm::vec4& color) {
        glBindFramebuffer(GL_FRAMEBUFFER, frames[frameIndex].framebuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, frames[frameIndex].camera);

        glViewport(viewport.x, viewport.y, viewport.width, viewport.height);

        glClearNamedFramebufferfv(frames[frameIndex].framebuffer, GL_COLOR, 0, glm::value_ptr(color));
        glClearNamedFramebufferfi(frames[frameIndex].framebuffer, GL_DEPTH_STENCIL, 0, 0, 0);
    }

    void endFrame() {
        glBlitNamedFramebuffer(frames[frameIndex].framebuffer, 0, 0, 0, displaySize.x, displaySize.y, 0, 0, displaySize.x, displaySize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        flipFrame();
    }

    void run() {
        glm::vec4 clear_color{ 0.45f, 0.55f, 0.60f, 1.00f };
        auto& io = ImGui::GetIO();

        using Clock = std::chrono::high_resolution_clock;

        auto last_time = Clock::now();
        while (!window->shouldClose()) {
            const auto current_time = Clock::now();
            const auto delta_time = current_time - std::exchange(last_time, current_time);
            const auto dt = std::chrono::duration<double>(delta_time).count();

            handleEvents();

            const auto windowSize = glm::vec2(window->getWindowSize());

            io.DisplaySize = ImVec2(windowSize.x, windowSize.y);
            if (windowSize.x > 0 && windowSize.y > 0) {
                const auto framebufferSize = glm::vec2(window->getFramebufferSize());
                const auto framebufferScale = windowSize / framebufferSize;

                io.DisplayFramebufferScale = ImVec2(framebufferScale.x, framebufferScale.y);
            }
            io.DeltaTime = static_cast<float>(dt);

            setupCamera();

            beginFrame(clear_color);
                imgui->begin();
                    ImGui::SetNextWindowPos(ImVec2(0, 0));
                    ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
                    ImGui::TextUnformatted(fmt::format("Application average {:.3f} ms/frame ({:.3f} FPS)", 1000.0f / io.Framerate, io.Framerate).c_str());
                    ImGui::End();
                imgui->end();
                imgui->flush();
            endFrame();
        }
    }
};

int main(int, char**) {
    Application app{"Application", 1280, 720 };
    app.run();
    return 0;
}

#include <Camera.hpp>
#include <ImGuiLayer.hpp>
#include <Application.hpp>

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

struct App : Application<App> {
    Transform transform{};
    std::unique_ptr<ImGuiLayer> imgui;
    std::unique_ptr<Camera> camera;

    std::vector<RenderFrame> frames{};
    int frameIndex = 0;

    Viewport viewport{};

    App(const char* title, int width, int height) : Application{title, width, height} {
        imgui = std::make_unique<ImGuiLayer>();

        camera = std::make_unique<Camera>();
        camera->setSize(width, height);

        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

        createFrames(width, height);
    }

    void handleEvent(const Event& event) {
        if (imgui->handleEvent(event)) {
            return;
        }

        matches(event,
            [this](const WindowResizeEvent& e) {
                for (auto& frame : frames) {
                    glDeleteTextures(1, &frame.color_attachment);
                    glCreateTextures(GL_TEXTURE_2D, 1, &frame.color_attachment);
                    glTextureStorage2D(frame.color_attachment, 1, GL_RGB8, e.width, e.height);
                    glTextureParameteri(frame.color_attachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTextureParameteri(frame.color_attachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    glDeleteRenderbuffers(1, &frame.depth_attachment);
                    glCreateRenderbuffers(1, &frame.depth_attachment);
                    glNamedRenderbufferStorage(frame.depth_attachment, GL_DEPTH32F_STENCIL8, e.width, e.height);

                    glDeleteFramebuffers(1, &frame.framebuffer);
                    glCreateFramebuffers(1, &frame.framebuffer);
                    glNamedFramebufferTexture(frame.framebuffer, GL_COLOR_ATTACHMENT0, frame.color_attachment, 0);
                    glNamedFramebufferRenderbuffer(frame.framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, frame.depth_attachment);

                    if (glCheckNamedFramebufferStatus(frame.framebuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                        fmt::print("Framebuffer is not complete!: {}\n", glGetError());
                    }
                }
            },
            [this](const FramebufferResizeEvent& e) {
                viewport = {0, 0, e.width, e.height};
            },
            [](const auto& _) {}
        );
    }

    void update(double dt) {
        const auto windowSize = glm::vec2(window->getWindowSize());

        auto& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(windowSize.x, windowSize.y);
        if (windowSize.x > 0 && windowSize.y > 0) {
            const auto framebufferSize = glm::vec2(window->getFramebufferSize());
            const auto framebufferScale = windowSize / framebufferSize;

            io.DisplayFramebufferScale = ImVec2(framebufferScale.x, framebufferScale.y);
        }
        io.DeltaTime = static_cast<float>(dt);

        setupCamera();
    }

    void renderFrame() {
        beginFrame(glm::vec4{ 0.45f, 0.55f, 0.60f, 1.00f });
        imgui->begin();

        auto& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
        ImGui::TextUnformatted(fmt::format("Application average {:.3f} ms/frame ({:.3f} FPS)", 1000.0f / io.Framerate, io.Framerate).c_str());
        ImGui::End();

        imgui->end();
        imgui->flush();
        endFrame();
    }

private:
    void createFrames(int width, int height) {
        viewport = {0, 0, width, height};

        frames.resize(2);
        for (auto& frame : frames) {
            glCreateBuffers(1, &frame.camera);
            glNamedBufferStorage(frame.camera, sizeof(CameraConstants), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
            frame.constants = glMapNamedBufferRange(frame.camera, 0, sizeof(CameraConstants), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

            glCreateTextures(GL_TEXTURE_2D, 1, &frame.color_attachment);
            glTextureStorage2D(frame.color_attachment, 1, GL_RGB8, width, height);
            glTextureParameteri(frame.color_attachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(frame.color_attachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glCreateRenderbuffers(1, &frame.depth_attachment);
            glNamedRenderbufferStorage(frame.depth_attachment, GL_DEPTH32F_STENCIL8, width, height);

            glCreateFramebuffers(1, &frame.framebuffer);
            glNamedFramebufferTexture(frame.framebuffer, GL_COLOR_ATTACHMENT0, frame.color_attachment, 0);
            glNamedFramebufferRenderbuffer(frame.framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, frame.depth_attachment);

            if (glCheckNamedFramebufferStatus(frame.framebuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                fmt::print("Framebuffer is not complete!: {}\n", glGetError());
            }
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
        glBlitNamedFramebuffer(frames[frameIndex].framebuffer, 0, viewport.x, viewport.y, viewport.width, viewport.height, viewport.x, viewport.y, viewport.width, viewport.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        frameIndex = static_cast<int>((static_cast<size_t>(frameIndex) + 1) % frames.size());
    }

    void setupCamera() {
        const auto projection_matrix = camera->getProjection();
        const auto transform_matrix = transform.getTransformMatrix();
        const auto camera_matrix = projection_matrix * transform_matrix;

        CameraConstants constants {
            .transform = camera_matrix,
            .position = glm::vec4(transform.position, 0.0f)
        };
        std::memcpy(frames[frameIndex].constants, &constants, sizeof(CameraConstants));
    }
};

int main(int, char**) {
    App app{"Application", 1280, 720};
    app.run();
    return 0;
}

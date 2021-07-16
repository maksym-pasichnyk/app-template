#include <Input.hpp>

#include <Application.hpp>
#include <AppPlatform.hpp>
#include <ImGuiLayer.hpp>
#include <Camera.hpp>
#include <Mesh.hpp>
#include <memory>
#include <array>

struct Transform {
    glm::vec2 rotation{};
    glm::vec3 position{};

    auto getRotationMatrix() const -> glm::mat4 {
        return getRotationMatrix(rotation);
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

    static auto getRotationMatrix(const glm::vec2& rotation) -> glm::mat4 {
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
};

struct CameraConstants {
    glm::mat4 transform;
    glm::vec4 position;
};

struct CameraUniform {
    GLuint handle;
    void* pointer;
};

struct BlockVertex {
    glm::vec3 pos;
    glm::u8vec4 col;
};

struct BlockRenderContext {
    std::vector<glm::i32> _indices{};
    std::vector<BlockVertex> _vertices{};

    std::span<const glm::i32> indices() const {
        return _indices;
    }

    std::span<const BlockVertex> vertices() const {
        return _vertices;
    }

    void quad() {
        _indices.emplace_back(_vertices.size() + 0);
        _indices.emplace_back(_vertices.size() + 1);
        _indices.emplace_back(_vertices.size() + 2);
        _indices.emplace_back(_vertices.size() + 0);
        _indices.emplace_back(_vertices.size() + 2);
        _indices.emplace_back(_vertices.size() + 3);
    }

    void vertex(const glm::vec3& pos, const glm::u8vec4& col) {
        _vertices.emplace_back(BlockVertex{.pos = pos, .col = col});
    }

    void cube(const glm::vec3& pos, float x0, float y0, float z0, float x1, float y1, float z1) {
        const auto min = pos + glm::vec3(x0, y0, z0) / 16.0f - glm::vec3(0.5f);
        const auto max = pos + glm::vec3(x1, y1, z1) / 16.0f - glm::vec3(0.5f);

        const auto p0 = glm::vec3{min.x, min.y, min.z};
        const auto p1 = glm::vec3{min.x, min.y, max.z};
        const auto p2 = glm::vec3{max.x, min.y, max.z};
        const auto p3 = glm::vec3{max.x, min.y, min.z};
        const auto p4 = glm::vec3{min.x, max.y, min.z};
        const auto p5 = glm::vec3{min.x, max.y, max.z};
        const auto p6 = glm::vec3{max.x, max.y, max.z};
        const auto p7 = glm::vec3{max.x, max.y, min.z};

        quad();
        vertex(p0, glm::u8vec4{0xFF, 0x00, 0x00, 0xFF});
        vertex(p4, glm::u8vec4{0xFF, 0x00, 0x00, 0xFF});
        vertex(p7, glm::u8vec4{0xFF, 0x00, 0x00, 0xFF});
        vertex(p3, glm::u8vec4{0xFF, 0x00, 0x00, 0xFF});

        quad();
        vertex(p3, glm::u8vec4{0x00, 0xFF, 0x00, 0xFF});
        vertex(p7, glm::u8vec4{0x00, 0xFF, 0x00, 0xFF});
        vertex(p6, glm::u8vec4{0x00, 0xFF, 0x00, 0xFF});
        vertex(p2, glm::u8vec4{0x00, 0xFF, 0x00, 0xFF});

        quad();
        vertex(p2, glm::u8vec4{0x00, 0x00, 0xFF, 0xFF});
        vertex(p6, glm::u8vec4{0x00, 0x00, 0xFF, 0xFF});
        vertex(p5, glm::u8vec4{0x00, 0x00, 0xFF, 0xFF});
        vertex(p1, glm::u8vec4{0x00, 0x00, 0xFF, 0xFF});

        quad();
        vertex(p1, glm::u8vec4{0xFF, 0x00, 0xFF, 0xFF});
        vertex(p5, glm::u8vec4{0xFF, 0x00, 0xFF, 0xFF});
        vertex(p4, glm::u8vec4{0xFF, 0x00, 0xFF, 0xFF});
        vertex(p0, glm::u8vec4{0xFF, 0x00, 0xFF, 0xFF});

        quad();
        vertex(p4, glm::u8vec4{0xFF, 0xFF, 0xFF, 0xFF});
        vertex(p5, glm::u8vec4{0xFF, 0xFF, 0xFF, 0xFF});
        vertex(p6, glm::u8vec4{0xFF, 0xFF, 0xFF, 0xFF});
        vertex(p7, glm::u8vec4{0xFF, 0xFF, 0xFF, 0xFF});

        quad();
        vertex(p1, glm::u8vec4{0xFF, 0xFF, 0xFF, 0xFF});
        vertex(p0, glm::u8vec4{0xFF, 0xFF, 0xFF, 0xFF});
        vertex(p3, glm::u8vec4{0xFF, 0xFF, 0xFF, 0xFF});
        vertex(p2, glm::u8vec4{0xFF, 0xFF, 0xFF, 0xFF});
    }
};

struct App : Application<App> {
    std::unique_ptr<ImGuiLayer> imgui{};
    std::vector<std::unique_ptr<RenderTarget>> frames{};
    int frameIndex = 0;

    /*****************************************************************************************************************/

    Input input{};
    Camera camera{};
    Viewport viewport{};
    Transform transform{};
    std::vector<CameraUniform> uniforms{};

    GLuint shader_handle;

    std::unique_ptr<Mesh> block_mesh;

    App(const char* title, int width, int height) : Application{title, width, height} {
        imgui = std::make_unique<ImGuiLayer>(*renderContext);

        CreateUniforms();
        CreateRenderTargets(width, height);

        auto vertex_source = AppPlatform::readFile("assets/default.vert").value();
        auto fragment_source = AppPlatform::readFile("assets/default.frag").value();

        shader_handle = renderContext->createShader(vertex_source, fragment_source);

        const std::array attributes {
            VertexArrayAttrib{0, 3, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(BlockVertex, pos))},
            VertexArrayAttrib{1, 4, GL_UNSIGNED_BYTE, GL_TRUE, static_cast<GLuint>(offsetof(BlockVertex, col))},
        };

        const std::array bindings {
            VertexArrayBinding{0, 0},
            VertexArrayBinding{1, 0}
        };

        BlockRenderContext ctx{};
        ctx.cube({}, 0, 0, 4, 16, 1, 12);
        ctx.cube({}, 1, 0, 3, 15, 1, 4);
        ctx.cube({}, 1, 0, 12, 15, 1, 13);
        ctx.cube({}, 1, 1, 4, 15, 4, 12);
        ctx.cube({}, 4, 4, 5, 12, 5, 12);
        ctx.cube({}, 6, 5, 5, 10, 10, 12);
        ctx.cube({}, 2, 10, 4, 14, 16, 12);
        ctx.cube({}, 14, 11, 4, 16, 15, 12);
        ctx.cube({}, 0, 11, 4, 2, 15, 12);
        ctx.cube({}, 3, 11, 3, 13, 15, 4);
        ctx.cube({}, 3, 11, 12, 13, 15, 13);

        block_mesh = std::make_unique<Mesh>(attributes, bindings, sizeof(BlockVertex), GL_STATIC_DRAW);
        block_mesh->SetIndices(ctx.indices());
        block_mesh->SetVertices(ctx.vertices());
    }

    void handleEvent(const Event& event) {
        matches(event,
            [this](const WindowResizeEvent& e) {
                CreateRenderTargets(e.width, e.height);
            },
            [this](const KeyEvent& e) {
                imgui->handleEvent(e);
                input.handleEvent(e);
            },
            [this](const MouseMoveEvent& e) {
                imgui->handleEvent(e);
                input.handleEvent(e);
            },
            [this](const MouseButtonEvent& e) {
                imgui->handleEvent(e);
                input.handleEvent(e);
            },
            [](const auto& _) {}
        );
    }

    void update(float dt) {
        input.update();

        auto& io = imgui->ctx->IO;
        io.DisplaySize.x = static_cast<float>(viewport.width);
        io.DisplaySize.y = static_cast<float>(viewport.height);
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
        io.DeltaTime = dt;
    }

    void renderFrame(float dt) {
        if (viewport.width <= 0 && viewport.height <= 0) {
            return;
        }

        transform.rotation.y = 10;
        transform.position.y = 2;
        transform.position.z = 10;

        auto renderTarget = BeginFrame(glm::vec4{ 0.45f, 0.55f, 0.60f, 1.00f });
        auto& io = imgui->begin();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
        ImGui::TextUnformatted(fmt::format("Application average {:.3f} ms/target ({:.3f} FPS)", 1000.0f / io.Framerate, io.Framerate).c_str());
        ImGui::End();

        imgui->end();
        imgui->flush();

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        glDepthFunc(GL_GREATER);
        glDisable(GL_BLEND);

        static float angle = 0;
        const auto rotation_matrix = Transform::getRotationMatrix({angle, 0});
        angle += dt * 50.0f;

        SetupCamera();

        glUseProgram(shader_handle);
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(rotation_matrix));
        glBindVertexArray(block_mesh->vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(block_mesh->index_count), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);

        EndFrame();

        glBlitNamedFramebuffer(renderTarget->framebuffer, 0, 0, 0, renderTarget->size.x, renderTarget->size.y, 0, 0, renderTarget->size.x, renderTarget->size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

private:
    void SetupCamera() {
        const auto projection_matrix = camera.getProjection();
        const auto transform_matrix = transform.getTransformMatrix();
        const auto camera_matrix = projection_matrix * transform_matrix;

        CameraConstants constants {
            .transform = camera_matrix,
            .position = glm::vec4(transform.position, 0.0f)
        };
        std::memcpy(uniforms[frameIndex].pointer, &constants, sizeof(CameraConstants));

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms[frameIndex].handle);
    }

    RenderTarget* BeginFrame(const glm::vec4& color) {
        auto renderTarget = frames[frameIndex].get();
        glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->framebuffer);
        glViewport(0, 0, renderTarget->size.x, renderTarget->size.y);

        glClearNamedFramebufferfv(renderTarget->framebuffer, GL_COLOR, 0, glm::value_ptr(color));
        glClearNamedFramebufferfi(renderTarget->framebuffer, GL_DEPTH_STENCIL, 0, 0, 0);

        return renderTarget;
    }

    void EndFrame() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        frameIndex = static_cast<int>((static_cast<size_t>(frameIndex) + 1) % frames.size());
    }

    void CreateUniforms() {
        uniforms.resize(2);
        for (auto& uniform : uniforms) {
            glCreateBuffers(1, &uniform.handle);
            glNamedBufferStorage(uniform.handle, sizeof(CameraConstants), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
            uniform.pointer = glMapNamedBufferRange(uniform.handle, 0, sizeof(CameraConstants), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        }
    }

    void CreateRenderTargets(int width, int height) {
        camera.setAspect(static_cast<float>(width) / static_cast<float>(height));

        viewport = {0, 0, width, height};
        frames.resize(2);
        for (auto& frame : frames) {
            if (width > 0 && height > 0) {
                frame = renderContext->createRenderTarget(width, height);
            } else {
                frame = nullptr;
            }
        }
    }
};

int main(int, char**) {
    App app{"Application", 1280, 720};
    app.run();
    return 0;
}

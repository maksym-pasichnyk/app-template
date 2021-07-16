#pragma once

#include <optional>
#include <glm/glm.hpp>

struct Viewport {
    int x;
    int y;
    int width;
    int height;
};

struct Camera {
    glm::mat4 getProjection() const noexcept {
        if (!_projection.has_value()) {
            const float f = 1.0f / glm::tan(glm::radians(_field_of_view) / 2.0f);
            _projection = glm::mat4{
                    f / _aspect, 0.0f, 0.0f, 0.0f,
                    0.0f, f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, _near_clipping_plane, 0.0f
            };
        }
        return *_projection;
    }

    void setAspect(float aspect) {
        _aspect = aspect;
        _projection.reset();
    }

private:
    mutable std::optional<glm::mat4> _projection;

    float _aspect = 0;
    float _field_of_view = 60.0f;
    float _near_clipping_plane = 0.15f;
};

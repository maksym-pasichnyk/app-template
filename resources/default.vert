#version 450

out gl_PerVertex {
    vec4 gl_Position;
};

layout (binding = 0) uniform CameraConstants {
    mat4 transform;
    vec3 position;
} constants;

layout(location = 0) uniform mat4 transform;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(location = 0) out struct {
    vec4 color;
} v_out;

void main() {
    gl_Position = constants.transform * transform * vec4(position, 1);

    v_out.color = color;
}
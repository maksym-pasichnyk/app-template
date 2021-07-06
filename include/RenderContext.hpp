#pragma once

#include <AppPlatform.hpp>
#include <fmt/format.h>
#include <string_view>
#include <GL/gl3w.h>
#include <string>

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

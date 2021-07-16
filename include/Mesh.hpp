#pragma once

#include <span>

struct VertexArrayAttrib {
    GLuint index;
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLuint offset;
};

struct VertexArrayBinding {
    GLuint index;
    GLuint binding;
};

struct Mesh {
    GLuint vao = GL_NONE;
    GLuint vbo = GL_NONE;
    GLuint ibo = GL_NONE;
    GLsizeiptr vbo_size = 0;
    GLsizeiptr ibo_size = 0;
    size_t index_count = 0;
    size_t vertex_count = 0;
    GLenum usage;

    Mesh(std::span<const VertexArrayAttrib> attributes, std::span<const VertexArrayBinding> bindings, GLsizei size, GLenum usage) : usage(usage) {
        glCreateVertexArrays(1, &vao);
        glCreateBuffers(1, &vbo);
        glCreateBuffers(1, &ibo);

        glVertexArrayElementBuffer(vao, ibo);
        glVertexArrayVertexBuffer(vao, 0, vbo, 0, size);

        for (const auto& attrib : attributes) {
            glEnableVertexArrayAttrib(vao, attrib.index);
            glVertexArrayAttribFormat(vao, attrib.index, attrib.size, attrib.type, attrib.normalized, attrib.offset);
        }

        for (const auto& binding : bindings) {
            glVertexArrayAttribBinding(vao, binding.index, binding.binding);
        }
    }

    ~Mesh() {
        glDeleteBuffers(1, &ibo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    template<typename T, size_t Extent = std::dynamic_extent>
    void SetVertices(std::span<T, Extent> vertices) {
        vertex_count = vertices.size();
        if (vertices.size_bytes() > vbo_size) {
            vbo_size = vertices.size_bytes();
            glNamedBufferData(vbo, vertices.size_bytes(), vertices.data(), usage);
        } else {
            glNamedBufferSubData(vbo, 0, vertices.size_bytes(), vertices.data());
        }
    }

    template<typename T, size_t Extent = std::dynamic_extent>
    void SetIndices(std::span<T, Extent> indices) {
        index_count = indices.size();
        if (indices.size_bytes() > ibo_size) {
            ibo_size = indices.size_bytes();
            glNamedBufferData(ibo, indices.size_bytes(), indices.data(), usage);
        } else {
            glNamedBufferSubData(ibo, 0, indices.size_bytes(), indices.data());
        }
    }
};
cmake_minimum_required(VERSION 3.19)
project(Template)

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS OFF)

add_definitions(-DGLM_FORCE_XYZW_ONLY)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/fmt")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glm")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/gl3w")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glfw")

add_library(imgui STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imconfig.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_internal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_tables.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_widgets.cpp"
)

add_library(imgui-glfw STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_glfw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_glfw.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_opengl3.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_opengl3.cpp"
)

target_include_directories(imgui-glfw PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/gl3w/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glfw/include"
)
target_link_libraries(imgui-glfw PRIVATE imgui imgui-glfw glfw gl3w)

add_executable(Template main.cpp include/Camera.hpp include/Event.hpp include/Mesh.hpp include/Window.hpp include/AppPlatform.hpp include/RenderContext.hpp include/utils/matches.hpp include/ImGuiLayer.hpp include/Application.hpp)
target_include_directories(Template PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/fmt/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/gl3w/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glfw/include"
)
target_compile_definitions(Template PRIVATE -DGLFW_INCLUDE_NONE)

target_link_libraries(Template PRIVATE imgui imgui-glfw glfw gl3w glm fmt)
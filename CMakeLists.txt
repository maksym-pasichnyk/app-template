cmake_minimum_required(VERSION 3.19)
project(Template)

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS OFF)

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

add_executable("${PROJECT_NAME}"
    main.cpp
    include/utils/matches.hpp
    include/Camera.hpp
    include/Event.hpp
    include/Mesh.hpp
    include/Window.hpp
    include/AppPlatform.hpp
    include/RenderContext.hpp
    include/ImGuiLayer.hpp
    include/Application.hpp
    include/Input.hpp
    include/Image.hpp
)

target_include_directories("${PROJECT_NAME}" PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/fmt/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/gl3w/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glfw/include"
)
target_compile_definitions("${PROJECT_NAME}" PRIVATE
    -DGLFW_INCLUDE_NONE
    -DIMGUI_DEFINE_MATH_OPERATORS
    -DGLM_FORCE_XYZW_ONLY
)
target_link_libraries("${PROJECT_NAME}" PRIVATE
    imgui
    glfw
    gl3w
    glm
    fmt
)
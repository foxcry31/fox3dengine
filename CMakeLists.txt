cmake_minimum_required(VERSION 3.22)

project(f3d C CXX)
include_directories(./include)
add_executable(f3de main.cpp src/glad.c f3d/camera.cpp f3d/texture.cpp f3d/shaders/EBO.cpp f3d/shaders/shader.cpp f3d/shaders/VAO.cpp f3d/shaders/VBO.cpp)
add_library(glad STATIC src/glad.c)
add_library(stb STATIC src/stb.cpp)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(f3de glfw glad stb)


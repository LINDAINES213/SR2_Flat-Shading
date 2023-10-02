cmake_minimum_required(VERSION 3.26)
project(SR2_Flat_Shading)

set(CMAKE_CXX_STANDARD 17)

set(SDL2_INCLUDE_DIR C:/libs/SDL/include)
set(SDL2_LIB_DIR C:/libs/SDL/lib/x64)

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})
include_directories("C:/MinGW/include/glm")

add_executable(SR2_Flat_Shading main.cpp
        color.h
        vertex.h
        face.h
        line.h
        triangle.h
        loadOBJ.h
        fragment.h
        uniform.h
        shaders.h
        framebuffer.h
)

target_link_libraries(${PROJECT_NAME} SDL2main SDL2)


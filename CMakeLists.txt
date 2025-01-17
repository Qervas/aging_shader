cmake_minimum_required(VERSION 3.15)
project(RayTracer
        VERSION 1.0.0
        DESCRIPTION "Compute Shader Raytracer"
        LANGUAGES CXX C)

option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_WARNINGS "Enable warnings" ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_WARNINGS "Enable warnings" ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(glad STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c
)
target_include_directories(glad PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

# Collect source files recursively
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

# Create main executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
        ${OPENGL_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        OpenGL::GL
        glfw
        glad
)

# Enable warnings
if(ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

# Create symlinks for shaders and textures directories
if(UNIX)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders
            ${CMAKE_BINARY_DIR}/shaders
    )
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_CURRENT_SOURCE_DIR}/textures
            ${CMAKE_BINARY_DIR}/textures
    )
else()
    # On Windows, we need admin privileges for symlinks, so we might want to fall back to copying
    # Or use mklink with proper permissions
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders
                ${CMAKE_BINARY_DIR}/shaders
        )
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/textures
                ${CMAKE_BINARY_DIR}/textures
        )
    endif()
endif()

# Optional: Add tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install rules (optional)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

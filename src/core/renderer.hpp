#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    void init();
    void render();
    void resize(int width, int height);
    GLuint getOutputTexture() const { return outputTexture; }
    static std::string loadShaderSource(const std::string& path);

private:
    int width, height;
    GLuint computeProgram;
    GLuint outputTexture;

    void createShaders();
    void createOutputTexture();
    GLuint compileComputeShader(const std::string& source);
};

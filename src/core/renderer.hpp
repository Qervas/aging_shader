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
    static std::string preprocessShader(const std::string& source, const std::string& shaderDir);
    static std::string getShaderDirectory(const std::string& shaderPath);
    void moveSphere(const glm::vec3& delta);
    const glm::vec3& getSpherePosition() const { return spherePosition; }
    void setRustLevel(float level) {
        rustLevel = glm::clamp(level, 0.0f, 1.0f);
    }
    float getRustLevel() const { return rustLevel; }
    void adjustRustLevel(float delta);

private:
    int width, height;
    GLuint computeProgram;
    GLuint outputTexture;
    glm::vec3 spherePosition{0.0f, 0.0f, -1.0f};
    GLint spherePositionLoc{-1};
    float rustLevel{0.0f};  // Initial rust level
    GLint rustLevelLoc{-1};

    void createShaders();
    void createOutputTexture();
    GLuint compileComputeShader(const std::string& source);
};

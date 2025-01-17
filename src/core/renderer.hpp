#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "image_loader.hpp"
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
    void setAge(float newAge) {
        age = glm::clamp(newAge, 0.0f, 1.0f);
    }
    float getAge() const { return age; }
    void adjustAge(float delta);
    void setFrameWidth(float width) {
        frameWidth = glm::clamp(width, 0.01f, 0.5f);
    }
    float getFrameWidth() const { return frameWidth; }

private:
    int width, height;
    GLuint computeProgram;
    GLuint outputTexture;
    glm::vec3 spherePosition{0.0f, 0.0f, -1.0f};
    GLint spherePositionLoc{-1};
    float rustLevel{0.0f}; // 0.0 = no rust, 1.0 = full rust
    GLint rustLevelLoc{-1};
    GLuint paintingTexture;
    GLint paintingTextureLoc;
    float age{0.0f};
    GLint ageLoc{-1};
    GLint frameWidthLoc{-1};
    float frameWidth{0.1f};
    void createShaders();
    void createOutputTexture();
    GLuint compileComputeShader(const std::string& source);
    void loadPaintingTexture(const std::string& path);

};

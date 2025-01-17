#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "core/camera.hpp"
#include "core/physics.hpp"
#include "core/scene.hpp"
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
    Physics& getPhysics() { return physics; }
    void setCameraPosition(const glm::vec3& pos) { camera.setPosition(pos); }
    Camera& getCamera() { return camera; }
    Scene& getScene() { return scene; }
    void setMoisture(float value) {
        moisture = glm::clamp(value, 0.0f, 1.0f);
    }
    float getMoisture() const { return moisture; }
    void adjustMoisture(float delta) {
        moisture = glm::clamp(moisture + delta, 0.0f, 1.0f);
    }
    void update(float deltaTime) {
        currentTime += deltaTime;
    }
    void setLightDirection(const glm::vec3& dir) {
        lightDirection = glm::normalize(dir);
    }

    void setLightIntensity(float intensity) {
        lightIntensity = glm::clamp(intensity, 0.0f, 10.0f);
    }
    void setupDramaticScene();
private:
    int width, height;
    GLuint computeProgram;
    GLuint outputTexture;
    float rustLevel{0.0f}; // 0.0 = no rust, 1.0 = full rust
    GLint rustLevelLoc{-1};
    GLuint paintingTexture;
    GLint paintingTextureLoc;
    float age{0.0f};
    GLint ageLoc{-1};
    GLint frameWidthLoc{-1};
    float frameWidth{0.1f};
    Physics physics;
    Camera camera;
    GLint cameraPositionLoc{-1};
    GLint cameraFrontLoc{-1};
    GLint cameraUpLoc{-1};
    Scene scene;
    std::vector<GLint> objectPositionLocs;
    GLint numObjectsLoc{-1};
    GLuint objectBuffer;
    std::vector<glm::vec4> objectData;
    float moisture{0.0f};
    GLint moistureLoc{-1};
    EnvironmentParams environment;
    float currentTime{0.0f};
    GLint iTimeLoc{-1};
    glm::vec3 lightDirection{-1.0f, -1.0f, -1.0f};
    float lightIntensity{1.0f};
    GLint lightDirLoc{-1};
    GLint lightIntensityLoc{-1};

    void updateEnvironment(float deltaTime);
    void createShaders();
    void createOutputTexture();
    GLuint compileComputeShader(const std::string& source);
    void loadPaintingTexture(const std::string& path);


};

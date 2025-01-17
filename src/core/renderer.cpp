#include "renderer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer(int w, int h)
    : width(w)
    , height(h)
    , camera(physics)
    , scene(physics)
{
    init();

    scene.addObject(ObjectType::GROUND, glm::vec3(0.0f, -1.0f, 0.0f), false);
    scene.addObject(ObjectType::SPHERE, glm::vec3(0.0f, 3.0f, -1.0f), true);
    scene.addObject(ObjectType::WALL, glm::vec3(0.0f, 0.0f, -5.0f), false);
    scene.addObject(ObjectType::RECTANGLE, glm::vec3(0.0f, 2.0f, -4.9f), false);
}

Renderer::~Renderer() {
    glDeleteProgram(computeProgram);
    glDeleteTextures(1, &outputTexture);
    glDeleteBuffers(1, &objectBuffer);

}

void Renderer::init() {
    createShaders();
    createOutputTexture();
    loadPaintingTexture("textures/painting.jpg");

    // Create and initialize object buffer
    glGenBuffers(1, &objectBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectBuffer);
    // Initialize with some space
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 100, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, objectBuffer);

    // Get uniform locations
    rustLevelLoc = glGetUniformLocation(computeProgram, "rustLevel");
    ageLoc = glGetUniformLocation(computeProgram, "age");
    frameWidthLoc = glGetUniformLocation(computeProgram, "frameWidth");
    cameraPositionLoc = glGetUniformLocation(computeProgram, "cameraPosition");
    cameraFrontLoc = glGetUniformLocation(computeProgram, "cameraFront");
    cameraUpLoc = glGetUniformLocation(computeProgram, "cameraUp");
    numObjectsLoc = glGetUniformLocation(computeProgram, "numObjects");
    moistureLoc = glGetUniformLocation(computeProgram, "moisture");
    lightDirLoc = glGetUniformLocation(computeProgram, "lightDirection");
    lightIntensityLoc = glGetUniformLocation(computeProgram, "lightIntensity");

    if (lightDirLoc == -1 || lightIntensityLoc == -1) {
        throw std::runtime_error("Could not find light uniforms");
    }
    if (moistureLoc == -1) {
        throw std::runtime_error("Could not find moisture uniform");
    }
    // Check all uniform locations
    if ( rustLevelLoc == -1 ||
        ageLoc == -1 || frameWidthLoc == -1 ||
        cameraPositionLoc == -1 || cameraFrontLoc == -1 ||
        cameraUpLoc == -1 || numObjectsLoc == -1) {
        throw std::runtime_error("Could not find shader uniforms");
    }
}

void Renderer::createOutputTexture() {
    glGenTextures(1, &outputTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

void Renderer::render() {
    // Update object data buffer
    objectData.clear();
    const auto& objects = scene.getObjects();
    for (const auto& obj : objects) {
        objectData.push_back(glm::vec4(obj.position, static_cast<float>(static_cast<int>(obj.type))));
    }

    // Update storage buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                   objectData.size() * sizeof(glm::vec4),
                   objectData.data());
    glUseProgram(computeProgram);
    // Update scene uniforms
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, paintingTexture);

    glUniform1i(numObjectsLoc, static_cast<GLint>(objects.size()));
    glUniform1f(rustLevelLoc, rustLevel);
    glUniform1f(ageLoc, age);
    glUniform1f(frameWidthLoc, frameWidth);
    glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(camera.getPosition()));
    glUniform3fv(cameraFrontLoc, 1, glm::value_ptr(camera.getFront()));
    glUniform3fv(cameraUpLoc, 1, glm::value_ptr(camera.getUp()));
    glUniform1f(moistureLoc, moisture);
    glUniform1f(iTimeLoc, currentTime);
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirection));
    glUniform1f(lightIntensityLoc, lightIntensity);

    // Dispatch compute shader
    glDispatchCompute((width + 7) / 8, (height + 7) / 8, 1);

    // Make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Renderer::adjustRustLevel(float delta) {
    rustLevel = glm::clamp(rustLevel + delta, 0.0f, 1.0f);
}


std::string Renderer::loadShaderSource(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Renderer::compileComputeShader(const std::string& source) {
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
    }

    return shader;
}

void Renderer::createShaders() {
    std::string computePath = "shaders/raytracer.comp";
    std::string computeSource = loadShaderSource(computePath);
    std::string preprocessedSource = preprocessShader(computeSource, getShaderDirectory(computePath));

    GLuint computeShader = compileComputeShader(preprocessedSource);

    computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);

    GLint success;
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(computeProgram, 512, NULL, infoLog);
        throw std::runtime_error("Shader program linking failed: " + std::string(infoLog));
    }

    glDeleteShader(computeShader);
}

std::string Renderer::getShaderDirectory(const std::string& shaderPath) {
    size_t lastSlash = shaderPath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return shaderPath.substr(0, lastSlash);
    }
    return "";
}

std::string Renderer::preprocessShader(const std::string& source, const std::string& shaderDir) {
    std::string result = source;
    std::string includeDirective = "#include";

    size_t pos = 0;
    while ((pos = result.find(includeDirective, pos)) != std::string::npos) {
        // Find the opening quote
        size_t startQuote = result.find("\"", pos);
        if (startQuote == std::string::npos) break;

        // Find the closing quote
        size_t endQuote = result.find("\"", startQuote + 1);
        if (endQuote == std::string::npos) break;

        // Extract the include filename
        std::string filename = result.substr(startQuote + 1, endQuote - startQuote - 1);
        std::string includePath = shaderDir + "/" + filename;

        // Load and preprocess the included file
        std::string includeContent = loadShaderSource(includePath);
        includeContent = preprocessShader(includeContent, getShaderDirectory(includePath));

        // Replace the include directive with the processed content
        result.replace(pos, endQuote - pos + 1, includeContent);

        // Move past the included content
        pos += includeContent.length();
    }

    return result;
}

void Renderer::loadPaintingTexture(const std::string& path) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        throw std::runtime_error("Failed to load painting texture: " + std::string(stbi_failure_reason()));
    }

    glGenTextures(1, &paintingTexture);
    glBindTexture(GL_TEXTURE_2D, paintingTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    // Get uniform location and bind to texture unit 1
    paintingTextureLoc = glGetUniformLocation(computeProgram, "paintingTexture");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, paintingTexture);
    glUniform1i(paintingTextureLoc, 1);
}

void Renderer::adjustAge(float delta) {
    age = glm::clamp(age + delta, 0.0f, 1.0f);
}

void Renderer::setupDramaticScene() {
    // Position camera for dramatic angle
    camera.setPosition(glm::vec3(3.0f, 2.0f, 3.0f));
    camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    // Set environment parameters
    setMoisture(0.8f);  // Very wet
    setRustLevel(0.7f); // Significantly rusted

    // Add dramatic lighting
    setLightIntensity(2.0f);
    setLightDirection(glm::vec3(-1.0f, -1.0f, -1.0f));
}

void Renderer::updateWeather(float deltaTime) {
    static float weatherCycle = 0.0f;
    weatherCycle += deltaTime * 0.1f;

    // Simulate weather changes
    float newMoisture = (sin(weatherCycle) * 0.5f + 0.5f);
    setMoisture(newMoisture);

    // Adjust lighting based on weather
    float intensity = 1.0f - (moisture * 0.5f);
    setLightIntensity(intensity);
}

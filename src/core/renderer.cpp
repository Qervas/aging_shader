#include "renderer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer(int w, int h) : width(w), height(h) {
    init();
}

Renderer::~Renderer() {
    glDeleteProgram(computeProgram);
    glDeleteTextures(1, &outputTexture);
}

void Renderer::init() {
    createShaders();
    createOutputTexture();
    spherePositionLoc = glGetUniformLocation(computeProgram, "spherePosition");
    if (spherePositionLoc == -1) {
        throw std::runtime_error("Could not find spherePosition uniform");
    }
    glUseProgram(computeProgram);
    glUniform3fv(spherePositionLoc, 1, glm::value_ptr(spherePosition));
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
    glUseProgram(computeProgram);
    glUniform3fv(spherePositionLoc, 1, glm::value_ptr(spherePosition));

    // Dispatch compute shader
    glDispatchCompute((width + 7) / 8, (height + 7) / 8, 1);

    // Make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Renderer::moveSphere(const glm::vec3& delta) {
    spherePosition += delta;
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

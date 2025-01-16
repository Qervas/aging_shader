#include "core/renderer.hpp"
#include <iostream>
#include <vector>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
GLuint createQuadProgram();
GLuint createQuadVAO();

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Compute Shader Raytracer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    try {
        Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);

        // Create and setup quad for displaying the texture
        GLuint quadVAO = createQuadVAO();
        GLuint quadProgram = createQuadProgram();

        // Set texture uniform
        glUseProgram(quadProgram);
        glUniform1i(glGetUniformLocation(quadProgram, "screenTexture"), 0);

        // Main rendering loop
        while (!glfwWindowShouldClose(window)) {
            processInput(window);

            // Render the scene using compute shader
            renderer.render();

            // Display the rendered texture
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(quadProgram);
            glBindVertexArray(quadVAO);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, renderer.getOutputTexture());

            glDrawArrays(GL_TRIANGLES, 0, 6);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        // Cleanup
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteProgram(quadProgram);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback([[maybe_unused]]  GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

GLuint createQuadVAO() {
    float quadVertices[] = {
        // positions        // texture coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return VAO;
}

GLuint createQuadProgram() {
    // Helper function to compile shader
    auto compileShader = [](const std::string& source, GLenum type) {
        GLuint shader = glCreateShader(type);
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
    };

    std::string vertPath = "shaders/quad.vert";
    std::string fragPath = "shaders/quad.frag";

    std::string vertSource = Renderer::loadShaderSource(vertPath);
    std::string fragSource = Renderer::loadShaderSource(fragPath);

    // Preprocess shaders
    vertSource = Renderer::preprocessShader(vertSource, Renderer::getShaderDirectory(vertPath));
    fragSource = Renderer::preprocessShader(fragSource, Renderer::getShaderDirectory(fragPath));

    GLuint vertShader = compileShader(vertSource, GL_VERTEX_SHADER);
    GLuint fragShader = compileShader(fragSource, GL_FRAGMENT_SHADER);

    // Create and link program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        throw std::runtime_error("Shader program linking failed: " + std::string(infoLog));
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return program;
}

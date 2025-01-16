#include "core/renderer.hpp"
#include <iostream>
#include <vector>

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;
const float MOVE_SPEED = 2.f;
const float RUST_CHANGE_SPEED = 0.5f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, Renderer& renderer, float deltaTime);
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
        float lastFrame = 0.0f;
        // Main rendering loop
        while (!glfwWindowShouldClose(window)) {
            float currentFrame = glfwGetTime();
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            processInput(window, renderer, deltaTime);

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

void processInput(GLFWwindow* window, Renderer& renderer, float deltaTime) {
    const float moveSpeed = MOVE_SPEED * deltaTime;
    const float rustSpeed = RUST_CHANGE_SPEED * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Movement controls with moveSpeed instead of MOVE_SPEED
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        renderer.moveSphere(glm::vec3(-moveSpeed, 0.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        renderer.moveSphere(glm::vec3(moveSpeed, 0.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        renderer.moveSphere(glm::vec3(0.0f, moveSpeed, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        renderer.moveSphere(glm::vec3(0.0f, -moveSpeed, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        renderer.moveSphere(glm::vec3(0.0f, 0.0f, -moveSpeed));
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        renderer.moveSphere(glm::vec3(0.0f, 0.0f, moveSpeed));
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        renderer.adjustRustLevel(rustSpeed);      // Increase rust
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        renderer.adjustRustLevel(-rustSpeed);     // Decrease rust
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

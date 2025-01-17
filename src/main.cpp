#include "core/renderer.hpp"
#include <iostream>
#include <vector>

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;
const float RUST_CHANGE_SPEED = 0.5f;
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, Renderer& renderer, float deltaTime);
GLuint createQuadProgram();
GLuint createQuadVAO();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

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
        renderer.setupDramaticScene();
        glfwSetWindowUserPointer(window, &renderer);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


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
            renderer.getPhysics().update(deltaTime);
            renderer.getScene().update(deltaTime);
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
    Camera& camera = renderer.getCamera();
    Physics& physics = renderer.getPhysics();
    glm::vec3 forward = glm::normalize(glm::vec3(camera.getFront().x, 0.0f, camera.getFront().z));
    glm::vec3 right = camera.getRight();
    glm::vec3 moveForce(0.0f);
    const float MOVE_FORCE = 30.0f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveForce += forward * MOVE_FORCE;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveForce -= forward * MOVE_FORCE;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveForce -= right * MOVE_FORCE;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveForce += right * MOVE_FORCE;
    if (physics.isCameraGrounded()) {
        physics.applyCameraForce(moveForce * deltaTime);
    } else {
        // Reduced air control
        physics.applyCameraForce(moveForce * deltaTime * 0.2f);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.jump();
    camera.update(deltaTime);

    // Rust and age controls (keeping existing functionality)
    const float rustSpeed = RUST_CHANGE_SPEED * deltaTime;
    const float ageSpeed = 0.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        renderer.setFrameWidth(renderer.getFrameWidth() - 0.1f * deltaTime);
        renderer.adjustAge(ageSpeed);
        renderer.adjustRustLevel(rustSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        renderer.setFrameWidth(renderer.getFrameWidth() + 0.1f * deltaTime);
        renderer.adjustAge(-ageSpeed);
        renderer.adjustRustLevel(-rustSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        renderer.adjustMoisture(0.5f * deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        renderer.adjustMoisture(-0.5f * deltaTime);
    }
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

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    renderer->getCamera().processMouseMovement(xoffset, yoffset);
}

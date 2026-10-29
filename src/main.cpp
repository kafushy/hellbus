#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.hpp"
#include "CubeMesh.hpp"
#include "Bus.hpp"
#include "Player.hpp"
#include "Camera.hpp"
#include "World.hpp"

#include <iostream>
#include <string>

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoords;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    uniform vec2 resolution;

    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoords;

    void main() {
        vec4 worldPos = model * vec4(aPos, 1.0);
        FragPos = worldPos.xyz;
        Normal = mat3(transpose(inverse(model))) * aNormal;
        TexCoords = aTexCoords;

        vec4 clipPos = projection * view * worldPos;

        // PS1 Pixel Snapping
        vec4 snapPos = clipPos;
        if (clipPos.w > 0.0) {
            snapPos.xyz = clipPos.xyz / clipPos.w;
            snapPos.xy = floor(resolution * snapPos.xy) / resolution;
            snapPos.xyz *= clipPos.w;
        }
        gl_Position = snapPos;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoords;

    uniform vec3 objectColor;
    uniform vec3 cameraPos;
    uniform vec3 fogColor;

    void main() {
        // Directional Light for 3D Depth
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(vec3(0.5, 1.2, 0.8));
        float diff = max(dot(norm, lightDir), 0.25); // Minimum 25% ambient lighting

        vec3 baseColor = objectColor;
        vec3 litColor = baseColor * diff;

        // Fog Calculation
        float dist = length(FragPos - cameraPos);
        float fogFactor = clamp((dist - 8.0) / 60.0, 0.0, 0.85);
        vec3 finalColor = mix(litColor, fogColor, fogFactor);

        FragColor = vec4(finalColor, 1.0);
    }
)";

Bus g_Bus;
Player g_Player;
World g_World;

float g_LastX = 640.0f, g_LastY = 360.0f;
bool g_FirstMouse = true;
bool g_EPressedLastFrame = false;
bool g_FPressedLastFrame = false;

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (g_FirstMouse) {
        g_LastX = xpos;
        g_LastY = ypos;
        g_FirstMouse = false;
    }

    float xoffset = xpos - g_LastX;
    float yoffset = g_LastY - ypos;
    g_LastX = xpos;
    g_LastY = ypos;

    g_Player.processLook(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    bool ePressed = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
    if (ePressed && !g_EPressedLastFrame) {
        g_Player.handleInteraction(g_Bus);
    }
    g_EPressedLastFrame = ePressed;

    bool fPressed = (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS);
    if (fPressed && !g_FPressedLastFrame) {
        g_World.update(deltaTime, g_Player, g_Bus, true);
    }
    g_FPressedLastFrame = fPressed;

    bool w = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    bool s = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    bool a = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    bool d = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    bool space = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    if (g_Player.state == PlayerState::DRIVING) {
        float oldYaw = g_Bus.yaw;
        g_Bus.update(deltaTime, w, s, a, d);
        float deltaYaw = g_Bus.yaw - oldYaw;

        g_Player.yaw += deltaYaw;
        g_Player.position = g_Bus.getDriverSeatPosition();
    } else if (g_Player.state == PlayerState::PASSENGER_CABIN) {
        g_Bus.applyFriction(deltaTime);
        glm::vec3 busForward(-sin(g_Bus.yaw), 0.0f, -cos(g_Bus.yaw));
        g_Bus.position += busForward * g_Bus.speed * deltaTime;

        // Walking & Jumping inside cabin aisle
        g_Player.updateInsideCabin(deltaTime, w, s, a, d, space, g_Bus);
    } else { // ON_FOOT
    g_Player.updateOnFoot(deltaTime, w, s, a, d, space, g_Bus);

    g_Bus.applyFriction(deltaTime);
    glm::vec3 busForward(-sin(g_Bus.yaw), 0.0f, -cos(g_Bus.yaw));
    g_Bus.position += busForward * g_Bus.speed * deltaTime;

    g_Bus.resolvePlayerCollision(g_Player.position, 0.5f);
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "HELLBUS", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);

    Shader shader(vertexShaderSource, fragmentShaderSource);
    CubeMesh cubeMesh;

    float lastFrame = 0.0f;
    glm::vec3 fogColor(0.08f, 0.05f, 0.09f);

    if (!g_Bus.loadAssets("assets/models/bus.obj", "assets/textures/bus_atlas.png")) {
    std::cerr << "Failed to load bus 3D model!" << std::endl;
    }


    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        std::string modeStr = (g_Player.state == PlayerState::DRIVING) ? "[DRIVING] (E to Stand Up)" : 
                              (g_Player.state == PlayerState::PASSENGER_CABIN) ? "[CABIN AISLE] (Space to Jump, E to Drive / Exit)" : 
                              "[ON FOOT] (E to Enter Bus, F Grab/Drop)";
        std::string title = "HELLBUS | " + modeStr + " | Speed: " + std::to_string((int)g_Bus.speed) + " km/h | Fuel: " + std::to_string((int)g_Bus.fuel) + "%";
        glfwSetWindowTitle(window, title.c_str());

        glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glm::mat4 projection = Camera::getProjectionMatrix(width, height);
        glm::mat4 view = Camera::getViewMatrix(g_Player);

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec2("resolution", glm::vec2((float)width / 2.0f, (float)height / 2.0f));
        shader.setVec3("cameraPos", g_Player.position);
        shader.setVec3("fogColor", fogColor);

        g_World.render(shader, cubeMesh, g_Player);
        g_Bus.render(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
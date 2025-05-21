#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Audio.hpp>
#include "Sound.h"

#include "Shader.h"
#include "Camera.h"
#include "model.h"
#include "flashlight.h"
#include "AABB.h"
#include "Player.h"
#include "Menu.h"

#include <iostream>
#include <vector>

struct ModelInstance {
    Model& model;
    glm::mat4 matrix;
};

struct PointLightState {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
    bool isOn;
};

struct Battery {
    glm::vec3 position;
    bool isActive;
    std::vector<glm::mat4> matrices;
};

std::vector<Battery> batteries;

Player player;

PointLightState pointLights[5] = {
    {glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), 1.0f, 0.0009f, 0.00032f, false},
    {glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), 1.0f, 0.0009f, 0.00032f, false},
    {glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), 1.0f, 0.0009f, 0.00032f, false},
    {glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), 1.0f, 0.0009f, 0.00032f, false},
    {glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), 1.0f, 0.0009f, 0.00032f, false},
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window, std::vector<AABB>& meshesAABB);
void RenderScene(Shader& shader, Model& model1, const glm::mat4& model1Matrix,
    Model& ourModel, const std::vector<glm::mat4>& swordMatrices,
    const std::vector<Battery>& batteries, Model& batteryModel,
    Model& flashlightModel, const glm::mat4& flashlightMatrix);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(vector<std::string> faces);


bool IsNearLight(const glm::vec3& cameraPos, const glm::vec3& lightPos, float threshold = 3.0f) {
    return glm::distance(cameraPos, lightPos) <= threshold;
}


const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1200;

Camera camera(glm::vec3(-546.0f, 7.0f, 628.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

Flashlight flashlight;
bool fKeyPressedLastFrame = false;

bool eKeyPressedLastFrame = false;
const float LIGHT_ACTIVATION_DISTANCE = 10.0f;
const float LIGHT_ACTIVATION_ANGLE = 15.0f;


float deltaTime = 0.0f;
float lastFrame = 0.0f;


SoundManager soundManager;
bool isWalking = false;
bool isJumping = false;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Maze", glfwGetPrimaryMonitor(), nullptr);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);


    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Menu menu(window);
    menu.AddButton("Restart", glm::vec2(SCR_WIDTH / 2 - 100, SCR_HEIGHT / 2 - 50), glm::vec2(200, 50), [&]() {
        // Перезапуск игры
        camera.SetPosition(glm::vec3(-546.0f, 7.0f, 628.0f));
        flashlight.BatteryLevel = 100.0f;

        for (int i = 0; i < 5; i++) {
            if (pointLights[i].isOn) {
                pointLights[i].isOn = false;
                pointLights[i].diffuse = glm::vec3(0.0f);
                pointLights[i].ambient = glm::vec3(0.0f);
                pointLights[i].specular = glm::vec3(0.0f);

                soundManager.stopAllFireSounds();
            }
        }

        for (auto& battery : batteries) {
            battery.isActive = true;
            // Восстанавливаем матрицы отрисовки если они были удалены
            if (battery.matrices.empty()) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, battery.position);
                model = glm::scale(model, glm::vec3(7.0f));
                battery.matrices.push_back(model);

                model = glm::mat4(1.0f);
                model = glm::translate(model, battery.position + glm::vec3(0.2f, 0.0f, -0.2f));
                model = glm::scale(model, glm::vec3(7.0f));
                battery.matrices.push_back(model);

                model = glm::mat4(1.0f);
                model = glm::translate(model, battery.position + glm::vec3(-0.2f, 0.0f, 0.2f));
                model = glm::scale(model, glm::vec3(7.0f));
                battery.matrices.push_back(model);
            }
        }

        player.Reset();

        menu.Toggle();
        });

    menu.AddButton("Exit", glm::vec2(SCR_WIDTH / 2 - 100, SCR_HEIGHT / 2 + 50), glm::vec2(200, 50), [&]() {
        glfwSetWindowShouldClose(window, true);
        });

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);


    if (!soundManager.loadSounds()) {
        std::cerr << "Failed to load sounds!" << std::endl;
        return -1;
    }


    Shader lightingShader("light_casters.vert", "light_casters.frag");
    Shader lightCubeShader("light_cube.vert", "light_cube.frag");
    Shader skyboxShader("skybox.vert", "skybox.frag");
	Shader shadowDepthShader("default.vert", "default.frag");

	Model model1("models/labirynth5.obj");
    Model ourModel("models/swordfornekit.obj");
	Model batteryModel("models/battery.obj");
	Model flashlightModel("models/Flashlight.obj");

    std::vector<glm::mat4> swordMatrices;
	std::vector<glm::mat4> batteryMatrices;

    glm::mat4 sword1 = glm::mat4(1.0f);
    sword1 = glm::translate(sword1, glm::vec3(-496.0f, 0.0f, -535.0f)); // Позиция
    sword1 = glm::scale(sword1, glm::vec3(2.0f)); // Масштаб
    swordMatrices.push_back(sword1);

    glm::mat4 sword2 = glm::mat4(1.0f);
    sword2 = glm::translate(sword2, glm::vec3(-490.0f, 0.0f, -73.0f));
    sword2 = glm::rotate(sword2, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Поворот
    sword2 = glm::scale(sword2, glm::vec3(2.0f));
    swordMatrices.push_back(sword2);

    glm::mat4 sword3 = glm::mat4(1.0f);
    sword3 = glm::translate(sword3, glm::vec3(-488.0f, 0.0f, 383.0f));
    sword3 = glm::scale(sword3, glm::vec3(2.0f));
    swordMatrices.push_back(sword3);

    glm::mat4 sword4 = glm::mat4(1.0f);
    sword4 = glm::translate(sword4, glm::vec3(458.0f, 0.0f, 370.0f));
    sword4 = glm::scale(sword4, glm::vec3(2.0f));
    swordMatrices.push_back(sword4);

    glm::mat4 sword5 = glm::mat4(1.0f);
    sword5 = glm::translate(sword5, glm::vec3(505.0f, 0.0f, -347.0f));
    sword5 = glm::scale(sword5, glm::vec3(2.0f));
    swordMatrices.push_back(sword5);

    batteries = {
        { glm::vec3(-486.0f, 3.0f, -535.0f), true, {} },
        { glm::vec3(-480.0f, 3.0f, -73.0f), true, {} },
        { glm::vec3(-478.0f, 3.0f, 383.0f), true, {} },
        { glm::vec3(468.0f, 3.0f, 370.0f), true, {} },
        { glm::vec3(510.0f, 3.0f, -347.0f), true, {} },
    };

    for (auto& battery : batteries) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, battery.position);
        model = glm::scale(model, glm::vec3(7.0f));
        battery.matrices.push_back(model);

        model = glm::mat4(1.0f);
        model = glm::translate(model, battery.position + glm::vec3(0.2f, 0.0f, -0.2f));
        model = glm::scale(model, glm::vec3(7.0f));
        battery.matrices.push_back(model);

        model = glm::mat4(1.0f);
        model = glm::translate(model, battery.position + glm::vec3(-0.2f, 0.0f, 0.2f));
        model = glm::scale(model, glm::vec3(7.0f));
        battery.matrices.push_back(model);
    }


    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    float skyboxVertices[] = {       
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glm::vec3 pointLightPositions[] = {
        glm::vec3(-496.0f, 3.0f, -535.0f),
        glm::vec3(-490.0f, 3.0f, -73.0f),
        glm::vec3(-488.0f,  3.0f, 383.0f),
        glm::vec3(458.0f, 3.0f, 370.0f),
        glm::vec3(505.0f, 3.0f, -347.0f)
    };

    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int modelVBO, modelVAO;
    glGenVertexArrays(1, &modelVAO);
    glGenBuffers(1, &modelVBO);

    glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(modelVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    lightingShader.use();
    lightingShader.setInt("diffuseMap", 0);
	lightingShader.setInt("specularMap", 1);
	lightingShader.setInt("normalMap", 2);

    vector<std::string> faces
    {
        ("textures/right.jpg"),
        ("textures/left.jpg"),
        ("textures/top.jpg"),
        ("textures/bottom.jpg"),
        ("textures/front.jpg"),
        ("textures/back.jpg")
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
    skyboxShader.setBool("ActiveCubeMap", false);


    while (!glfwWindowShouldClose(window))
    {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        std::vector<AABB> allAABB;

        auto model1AABB = model1.GetMeshesAABB(glm::vec3(300.0f, 150.0f, 300.0f), glm::vec3(0.0f, -25.0f, 0.0f));
        allAABB.insert(allAABB.end(), model1AABB.begin(), model1AABB.end());

        for (const auto& matrix : swordMatrices) {
            auto swordAABB = ourModel.GetMeshesAABB(glm::vec3(2.0f), glm::vec3(0.0f, 3.0f, 0.0f));
            for (auto& box : swordAABB) {
                box.min = glm::vec3(matrix * glm::vec4(box.min, 1.0f));
                box.max = glm::vec3(matrix * glm::vec4(box.max, 1.0f));
                allAABB.push_back(box);
            }
        }

        for (const auto& matrix : batteryMatrices) {
            auto batteryAABB = ourModel.GetMeshesAABB(glm::vec3(2.0f), glm::vec3(0.0f, 3.0f, 0.0f));
            for (auto& box : batteryAABB) {
                box.min = glm::vec3(matrix * glm::vec4(box.min, 1.0f));
                box.max = glm::vec3(matrix * glm::vec4(box.max, 1.0f));
                allAABB.push_back(box);
            }
        }

        processInput(window, allAABB);
        camera.UpdatePhysics(deltaTime, soundManager);

        static bool escPressedLastFrame = false;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressedLastFrame) {
            menu.Toggle();
            escPressedLastFrame = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
            escPressedLastFrame = false;
        }

        if (menu.IsActive()) {
            menu.ProcessInput();
            menu.Render();
            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        bool eKeyPressedLastFrame = false;
        const float LIGHT_ACTIVATION_DISTANCE = 9.0f;
        const float LIGHT_ACTIVATION_ANGLE = 15.0f;

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !eKeyPressedLastFrame) {
            for (int i = 0; i < 5; i++) {
                float distanceLight = glm::distance(camera.Position, pointLightPositions[i]);
                if (distanceLight <= LIGHT_ACTIVATION_DISTANCE &&
                    camera.IsLookingAt(pointLightPositions[i], LIGHT_ACTIVATION_ANGLE) && pointLights[i].isOn == false) {

                    pointLights[i].isOn = true;
                    player.RegisterFireActivation();
                    soundManager.playSound(SoundManager::BORNFIRE, 80.0f);
                    soundManager.playFireSound(pointLightPositions[i], 20.0f, true);
                    glm::vec3 newDiffuse = pointLights[i].isOn ? glm::vec3(1.0f, 0.35f, 0.0f) : glm::vec3(0.0f);

					pointLights[i].diffuse = newDiffuse;
                    pointLights[i].ambient = newDiffuse;
                    pointLights[i].specular = newDiffuse;

                }
            }

            for (auto& battery : batteries) {
                if (battery.isActive) {
                    float distance = glm::distance(camera.Position, battery.position);
                    if (distance <= LIGHT_ACTIVATION_DISTANCE &&
                        camera.IsLookingAt(battery.position, LIGHT_ACTIVATION_ANGLE))
                    {
                        soundManager.playSound(SoundManager::BATTERY, 80.0f);
                        flashlight.Charge();
                        battery.isActive = false;
                        battery.matrices.clear();
                        break;
                    }
                }
            }

            eKeyPressedLastFrame = true;
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        float near_plane = 4.0f, far_plane = 2000.0f;
        glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(flashlight.Position, flashlight.Position + flashlight.Direction, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        shadowDepthShader.use();
        shadowDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glm::mat4 model1Matrix = glm::mat4(1.0f);
        model1Matrix = glm::translate(model1Matrix, glm::vec3(0.0f, -25.0f, 0.0f));
        model1Matrix = glm::scale(model1Matrix, glm::vec3(300.0f, 150.0f, 300.0f));
        lightingShader.setMat4("model", model1Matrix);

        glm::mat4 flashlightMatrix = glm::mat4(1.0f);
        flashlightMatrix = glm::translate(flashlightMatrix, flashlight.Position);
        flashlightMatrix = glm::rotate(flashlightMatrix, glm::radians(-camera.Yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        flashlightMatrix = glm::rotate(flashlightMatrix, glm::radians(-camera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f));

        flashlightMatrix = glm::rotate(flashlightMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        flashlightMatrix = glm::scale(flashlightMatrix, glm::vec3(8.0f));

        lightingShader.setMat4("model", flashlightMatrix);

        std::vector<ModelInstance> models = {
            {model1, model1Matrix}
        };

        RenderScene(
            shadowDepthShader,
            model1,
            model1Matrix,
            ourModel,
            swordMatrices,
            batteries,
            batteryModel,
            flashlightModel,
            flashlightMatrix
        );

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);

        lightingShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        lightingShader.setInt("shadowMap", 3);

        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        for (int i = 0; i < 5; i++) {
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", pointLights[i].diffuse);
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLightPositions[i]);
            lightingShader.setVec3("pointLightPositions[" + std::to_string(i) + "]", pointLightPositions[i]);
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", pointLights[i].ambient);
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].specular", pointLights[i].specular);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].constant", pointLights[i].constant);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].linear", pointLights[i].linear);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", pointLights[i].quadratic);
        }
        lightingShader.setVec3("spotLight.position", flashlight.Position);
        lightingShader.setVec3("spotLight.direction", flashlight.Direction);
        lightingShader.setVec3("spotLightDirection", flashlight.Direction);
        lightingShader.setVec3("lightPos", flashlight.Position);
        lightingShader.setVec3("spotLight.ambient", flashlight.Ambient);
        lightingShader.setVec3("spotLight.diffuse", flashlight.Diffuse);
        lightingShader.setVec3("spotLight.specular", flashlight.Specular);
        lightingShader.setFloat("spotLight.constant", flashlight.Constant);
        lightingShader.setFloat("spotLight.linear", flashlight.Linear);
        lightingShader.setFloat("spotLight.quadratic", flashlight.Quadratic);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(flashlight.CutOff)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(flashlight.OuterCutOff)));
		lightingShader.setBool("spotLight.state", flashlight.State);

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyPressedLastFrame) {
            if (flashlight.State) {
                flashlight.TurnOff();
                soundManager.playSound(SoundManager::FLASHLIGHT, 80.0f);
            }
            else {
                flashlight.TurnOn();
                soundManager.playSound(SoundManager::FLASHLIGHT, 80.0f);
            }
            fKeyPressedLastFrame = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
            fKeyPressedLastFrame = false;
        }

        soundManager.setListenerPosition(camera.Position.x, camera.Position.y, camera.Position.z);
        soundManager.setListenerDirection(camera.Front.x, camera.Front.y, camera.Front.z);

        soundManager.updateSoundPosition(SoundManager::WALK,
            camera.Position.x,
            camera.Position.y - 1.0f,
            camera.Position.z);

        soundManager.updateSoundPosition(SoundManager::JUMP,
            camera.Position.x,
            camera.Position.y - 1.0f,
            camera.Position.z);

        std::vector<glm::vec3> activeFirePositions;
        for (int i = 0; i < 5; i++) {
            if (pointLights[i].isOn) {
                activeFirePositions.push_back(pointLightPositions[i]);
            }
        }

        soundManager.updateFireSoundPositions(activeFirePositions);

        player.Update(deltaTime, flashlight, camera, activeFirePositions);

        if (player.sleepState == Player::SLEEP) {
            float intensity = (sin(glfwGetTime() * 10.0f) * 0.5f + 0.5f) * 0.3f + 0.2f;
            flashlight.Diffuse = glm::vec3(intensity);
        }
        else if (flashlight.State) {
            flashlight.Diffuse = glm::vec3(1.0f);
        }

        player.HandleHKeyPress(window, flashlight, camera, activeFirePositions);

        bool currentlyWalking = camera.IsMoving();
        if (currentlyWalking != isWalking) {
            isWalking = currentlyWalking;
            if (isWalking) {
                soundManager.playSound(SoundManager::WALK, 70.0f, true);
            }
            else {
                soundManager.stopSound(SoundManager::WALK);
            }
        }

        bool currentJumping = camera.IsJumping;

        if (currentJumping != isJumping) {
            isJumping = currentJumping;
            if (isJumping) {
                soundManager.playSound(SoundManager::JUMP, 80.0f);
            }
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        glBindVertexArray(modelVAO);
        
        RenderScene(
            lightingShader,
            model1,
            model1Matrix,
            ourModel,
            swordMatrices,
            batteries,
            batteryModel,
            flashlightModel,
            flashlightMatrix
        );
        
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);


        
        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < 5; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.05f));
            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        if (activeFirePositions.size() == 5) {
            skyboxShader.setBool("ActiveCubeMap", true);
        }
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &modelVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &modelVBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1, &depthMapFBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, std::vector<AABB>& meshesAABB)
{
    static bool escPressedLastFrame = false;
    bool escPressedNow = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    if (escPressedNow && !escPressedLastFrame) {
        escPressedLastFrame = true;
        return;
    }
    escPressedLastFrame = escPressedNow;

    if (!camera.neckAnimation.isActive) {
        camera.ProcessKeyboard(FORWARD, glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
        camera.ProcessKeyboard(BACKWARD, glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
        camera.ProcessKeyboard(LEFT, glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
        camera.ProcessKeyboard(RIGHT, glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
    }
    else {
        camera.moveForward = false;
        camera.moveBackward = false;
        camera.moveLeft = false;
        camera.moveRight = false;
        camera.isMoving = false;
    }

    static bool spaceWasPressed = false;
    bool spaceIsPressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (spaceIsPressed && !spaceWasPressed) {
        camera.ProcessKeyboard(JUMP, true);
    }
    spaceWasPressed = spaceIsPressed;

    flashlight.Position = glm::vec3(camera.Position.x - camera.Front.z * 2.0f,
        camera.Position.y + 1.0f,
        camera.Position.z + camera.Front.x * 2.0f);
    flashlight.Position += camera.Front * 4.0f;
    flashlight.Direction = camera.Front;
    camera.UpdatePosition(deltaTime, meshesAABB);
    
    camera.UpdateIdleAnimation(deltaTime);
    flashlight.UpdateBattery(deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{

    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}



void RenderScene(Shader& shader, Model& model1, const glm::mat4& model1Matrix,
    Model& ourModel, const std::vector<glm::mat4>& swordMatrices,
    const std::vector<Battery>& batteries, Model& batteryModel,
    Model& flashlightModel, const glm::mat4& flashlightMatrix)
{

    shader.setMat4("model", model1Matrix);
    model1.Draw(shader);

    shader.setMat4("model", flashlightMatrix);
    flashlightModel.Draw(shader);

    for (const auto& matrix : swordMatrices) {
        shader.setMat4("model", matrix);
        ourModel.Draw(shader);
    }

    for (const auto& battery : batteries) {
        if (battery.isActive) {
            for (const auto& matrix : battery.matrices) {
                shader.setMat4("model", matrix);
                batteryModel.Draw(shader);
            }
        }
    }
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
#include "Menu.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>



Menu::Menu(GLFWwindow* window) : window(window), isActive(true) {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";

    buttonShaderProgram = CompileShader(vertexShaderSource, fragmentShaderSource);

    textShaderProgram = CompileShader(vertexShaderSource, R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 textColor;
        void main() {
            FragColor = vec4(textColor, 1.0);
        }
    )");

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Menu::~Menu() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(buttonShaderProgram);
    glDeleteProgram(textShaderProgram);
}

void Menu::AddButton(const std::string& text, const glm::vec2& position, const glm::vec2& size, std::function<void()> action) {
    buttons.push_back({ position, size, text, action });
}

void Menu::Render() {
    if (!isActive) return;

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);

    glUseProgram(buttonShaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(buttonShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3f(glGetUniformLocation(buttonShaderProgram, "color"), 0.0f, 0.0f, 0.0f);

    float vertices[] = {
        0.0f, 0.0f,
        (float)width, 0.0f,
        (float)width, (float)height,
        0.0f, 0.0f,
        (float)width, (float)height,
        0.0f, (float)height
    };

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for (const auto& button : buttons) {
        RenderButton(button);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void Menu::RenderButton(const Button& button) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);

    glUseProgram(buttonShaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(buttonShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3f(glGetUniformLocation(buttonShaderProgram, "color"), 0.2f, 0.2f, 0.2f);

    float x = button.position.x;
    float y = button.position.y;
    float w = button.size.x;
    float h = button.size.y;

    float vertices[] = {
        x, y,
        x + w, y,
        x + w, y + h,
        x, y,
        x + w, y + h,
        x, y + h
    };

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    RenderText(button.text, x + w / 2 - button.text.length() * 10, y + h / 2 - 10, 0.5f, glm::vec3(1.0f));
}

void Menu::RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color) {

    glUseProgram(textShaderProgram);
    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), color.x, color.y, color.z);


}

void Menu::ProcessInput() {
    if (!isActive) return;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        for (auto& button : buttons) {
            if (xpos >= button.position.x && xpos <= button.position.x + button.size.x &&
                ypos >= button.position.y && ypos <= button.position.y + button.size.y) {
                button.action();
                break;
            }
        }
    }
}

void Menu::Toggle() {
    isActive = !isActive;
    if (isActive) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

bool Menu::IsActive() const {
    return isActive;
}

unsigned int Menu::CompileShader(const char* vertexSource, const char* fragmentSource) {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
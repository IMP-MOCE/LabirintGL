#ifndef MENU_H
#define MENU_H

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <functional>

class Menu {
public:
    struct Button {
        glm::vec2 position;
        glm::vec2 size;
        std::string text;
        std::function<void()> action;
    };

    Menu(GLFWwindow* window);
    ~Menu();

    void AddButton(const std::string& text, const glm::vec2& position, const glm::vec2& size, std::function<void()> action);
    void Render();
    void ProcessInput();
    void Toggle();
    bool IsActive() const;

private:
    GLFWwindow* window;
    std::vector<Button> buttons;
    bool isActive;
    unsigned int VAO, VBO;
    unsigned int textShaderProgram;
    unsigned int buttonShaderProgram;

    void RenderButton(const Button& button);
    void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color);
    unsigned int CompileShader(const char* vertexSource, const char* fragmentSource);
};

#endif
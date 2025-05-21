#ifndef PLAYER_H
#define PLAYER_H
#include <chrono>
#include <random>
#include <GLFW/glfw3.h>
#include "flashlight.h"
class Player {
public:
    enum State { AWAKE, SLEEP };
    State sleepState;
    float active = 0.0f;

    Player() : sleepTimer(0.0f), sleepCooldown(0.0f),
        activatedFires(0), nextSleepTrigger(30.0f) {
        std::random_device rd;
        gen = std::mt19937(rd());
    }

    void Update(float deltaTime, Flashlight& flashlight,
        Camera& camera, const std::vector<glm::vec3>& firePositions) {
        if (sleepState == AWAKE) {
            sleepCooldown += deltaTime;
            if (sleepCooldown >= nextSleepTrigger) {
                StartSleep(flashlight, camera);
                sleepCooldown = 0.0f;
                GenerateNewSleepTrigger();
            }
        }
        else {
            sleepTimer += deltaTime;
            if (sleepTimer >= 7.0f) {
                CancelSleep(flashlight, camera, firePositions);
            }
        }
    }

    void HandleHKeyPress(GLFWwindow* window, Flashlight& flashlight, Camera& camera, const std::vector<glm::vec3>& firePositions) {
        static bool hKeyPressedLastFrame = false;
        bool hKeyPressedNow = glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS;

        if (hKeyPressedNow && !hKeyPressedLastFrame) {
            if (!camera.neckAnimation.isActive) {
                camera.StartNeckAnimation();
            }
        }
        hKeyPressedLastFrame = hKeyPressedNow;

        if (camera.neckAnimation.animationCompleted) {
            if (sleepState == SLEEP) {
                CompleteSleep(flashlight, camera);
            }
            else {
                EmergencyTeleport(camera, firePositions);
                flashlight.BatteryLevel *= 0.5f;
            }
            camera.neckAnimation.animationCompleted = false;
            active = 0.0f;
            return;
        }
        
    }

    void RegisterFireActivation() {
        activatedFires++;
        GenerateNewSleepTrigger();
    }

    void Reset() {
        activatedFires = 0;
        sleepState = AWAKE;
        sleepTimer = 0.0f;
        sleepCooldown = 0.0f;
        GenerateNewSleepTrigger();
    }

private:
    float sleepTimer;
    float sleepCooldown;
    float nextSleepTrigger;
    int activatedFires;
    glm::vec3 savedPosition;
    float savedBattery;

    std::mt19937 gen;

    void GenerateNewSleepTrigger() {
        float baseInterval = 30.0f / (1 + activatedFires * 0.5f);
        if (activatedFires == 5) {
            float baseInterval = 999999999999.9f;
        }
        std::uniform_real_distribution<> distr(baseInterval * 0.7f, baseInterval * 1.3f);
        nextSleepTrigger = distr(gen);
    }

    void EmergencyTeleport(Camera& camera, const std::vector<glm::vec3>& firePositions) {
        glm::vec3 targetPos = firePositions.empty() ?
            glm::vec3(-546.0f, 7.0f, 628.0f) :
            firePositions.back() + glm::vec3(3.0f, 4.0f, 0.0f);

        camera.SetPosition(targetPos);
    }

    void StartSleep(Flashlight& flashlight, Camera& camera) {
        sleepState = SLEEP;
        sleepTimer = 0.0f;
        savedPosition = camera.Position;
        savedBattery = flashlight.BatteryLevel;
    }

    void CompleteSleep(Flashlight& flashlight, Camera& camera) {
        sleepState = AWAKE;
        camera.Position = savedPosition;
        flashlight.BatteryLevel = savedBattery;
        sleepTimer = 0.0f;
    }

    void CancelSleep(Flashlight& flashlight, Camera& camera, const std::vector<glm::vec3>& firePositions) {
        sleepState = AWAKE;

        if (!firePositions.empty()) {
            camera.Position = firePositions.back() + glm::vec3(3.0f, 4.0f, 0.0f);
        }
        else {
            camera.Position = glm::vec3(-546.0f, 7.0f, 628.0f);
        }
        sleepTimer = 0.0f;
        flashlight.BatteryLevel *= 0.5f;
    }
};
#endif
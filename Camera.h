#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "AABB.h"
#include <vector>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    JUMP
};


const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 50.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
const float GRAVITY = -30.6f;
const float JUMP_VELOCITY = 10.0f;


class Camera
{
public:

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw; 
    float Pitch; 

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    bool IsJumping;
    float VelocityY;
    float JumpTurnSpeedFactor = 0.3f;
    float MaxJumpYawChange = 45.0f;
    float currentJumpYawChange = 0.0f;
    glm::vec3 jumpInitialFront;

    bool moveForward;
    bool moveBackward;
    bool moveLeft;
    bool moveRight;
    bool CanChangeDirection;

    float walkBobAmount = 0.05f;    
    float walkBobSpeed = 5.0f;      
    float walkBobTimer = 0.0f;     
    bool isMoving = false;         
    glm::vec3 basePosition;         

    struct {
        bool isActive = false;
        float timer = 0.0f;
        glm::vec3 startPosition;
        float startYaw;
        float startPitch;
        bool animationCompleted = false;
    } neckAnimation;


    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM),
        IsJumping(false), VelocityY(0.0f), moveForward(false), moveBackward(false), moveLeft(false), moveRight(false), CanChangeDirection(true)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), IsJumping(false), VelocityY(0.0f)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        float tiltAngle = 0.0f;
        if (isMoving) {
            tiltAngle = cos(walkBobTimer * 2.0f) * 0.5f;
        }

        glm::mat4 tilt = glm::rotate(glm::mat4(1.0f), glm::radians(tiltAngle), Front);

        glm::vec3 tiltedUp = glm::vec3(tilt * glm::vec4(Up, 0.0f));

        return glm::lookAt(Position, Position + Front, tiltedUp);
    }
    void ProcessKeyboard(Camera_Movement direction, bool pressed) {
        if (IsJumping) return;
        switch (direction) {
        case FORWARD:  moveForward = pressed; break;
        case BACKWARD: moveBackward = pressed; break;
        case LEFT:     moveLeft = pressed; break;
        case RIGHT:    moveRight = pressed; break;
        case JUMP:
            if (pressed && !IsJumping) {
                IsJumping = true;
                VelocityY = JUMP_VELOCITY;
                jumpInitialFront = Front;
                currentJumpYawChange = 0.0f;
            }
            break;
        }
    }

    void UpdatePosition(float deltaTime, const std::vector<AABB>& meshesAABB) {


        if (!isMoving) {
            basePosition = Position;
        }

        isMoving = moveForward || moveBackward || moveLeft || moveRight;

        float velocity = MovementSpeed * deltaTime;
        glm::vec3 horizontalFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        glm::vec3 moveDirection(0.0f);

        if (moveForward)  moveDirection += horizontalFront;
        if (moveBackward) moveDirection -= horizontalFront;
        if (moveLeft)     moveDirection -= Right;
        if (moveRight)    moveDirection += Right;

        if (glm::length(moveDirection) > 0.0f) {
            moveDirection = glm::normalize(moveDirection);
            glm::vec3 newPosition = Position + moveDirection * velocity;

            if (IsJumping) {
                velocity *= 0.0f;
            }

            bool collisionDetected = false;
            for (const auto& aabb : meshesAABB) {
                if (CheckCollision(newPosition, aabb)) {
                    collisionDetected = true;
                    break;
                }
            }

            if (!collisionDetected) {
                Position = newPosition;
            }
            else {
                glm::vec3 tempPosition = Position;
                tempPosition.x = newPosition.x;
                bool collisionX = false;
                for (const auto& aabb : meshesAABB) {
                    if (CheckCollision(tempPosition, aabb)) {
                        collisionX = true;
                        break;
                    }
                }
                if (!collisionX) Position.x = newPosition.x;

                tempPosition = Position;
                tempPosition.z = newPosition.z;
                bool collisionZ = false;
                for (const auto& aabb : meshesAABB) {
                    if (CheckCollision(tempPosition, aabb)) {
                        collisionZ = true;
                        break;
                    }
                }
                if (!collisionZ) Position.z = newPosition.z;
            }
        }
        if (isMoving && !IsJumping) {
            walkBobTimer += deltaTime * walkBobSpeed;
            float bobOffset = sin(walkBobTimer) * walkBobAmount;

            Position.y = basePosition.y + bobOffset;
        }
        else {
            Position.y = glm::mix(Position.y, basePosition.y, deltaTime * 5.0f);
            walkBobTimer = 0.0f;
        }
    }

    void UpdatePhysics(float deltaTime, SoundManager& soundManager) {

        if (neckAnimation.isActive) {
            neckAnimation.timer += deltaTime;

            if (neckAnimation.timer <= 0.3f) {
                soundManager.playSound(SoundManager::NECK, 1500.0f);
                Yaw = neckAnimation.startYaw + 90.0f * (neckAnimation.timer / 0.3f);
                Pitch = neckAnimation.startPitch - 45.0f * (neckAnimation.timer / 0.3f);
            }
            else if (neckAnimation.timer <= 0.8f && neckAnimation.timer > 0.3f) {
                float fallProgress = (neckAnimation.timer - 0.3f) / 0.3f;
                Position.y = neckAnimation.startPosition.y - 3.0f * pow(fallProgress, 2);
            }
            else if (neckAnimation.timer <= 2.5f) {
                
            }
            else {
                neckAnimation.isActive = false;
                neckAnimation.animationCompleted = true;
                ResetCamera();
            }
            updateCameraVectors();
            return;
        }

        if (IsJumping) {
            Position.y += VelocityY * deltaTime;
            VelocityY += GRAVITY * deltaTime;
            if (Position.y <= 7.0f) {
                Position.y = 7.0f;
                IsJumping = false;
            }
        }
    }

    void UpdateIdleAnimation(float deltaTime) {
        if (!isMoving && !IsJumping) {
            static float idleTimer = 0.0f;
            idleTimer += deltaTime;
            float breathOffset = sin(idleTimer * 0.5f) * 0.0001f;
            Position.y = basePosition.y + breathOffset;
        }
    }

    bool IsMoving() const {
        return (moveForward || moveBackward || moveLeft || moveRight) && (IsJumping == false);
    }

    void SetPosition(const glm::vec3& newPos) {
        Position = newPos;
        basePosition = newPos;
    }
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        if (neckAnimation.isActive) return;

        if (IsJumping) {
            xoffset *= MouseSensitivity * JumpTurnSpeedFactor;
            yoffset *= MouseSensitivity * JumpTurnSpeedFactor;

            float potentialYawChange = currentJumpYawChange + abs(xoffset);
            if (potentialYawChange > MaxJumpYawChange) {
                xoffset = 0.0f;
            }
            else {
                currentJumpYawChange = potentialYawChange;
            }
        }
        else {
            xoffset *= MouseSensitivity;
            yoffset *= MouseSensitivity;
        }

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }
        updateCameraVectors();
    }

    bool IsLookingAt(const glm::vec3& point, float maxAngle) const {
        glm::vec3 directionToPoint = glm::normalize(point - Position);
        float angle = glm::degrees(glm::acos(glm::dot(Front, directionToPoint)));
        return angle <= maxAngle;
    }

    void StartNeckAnimation() {
        neckAnimation.isActive = true;
        neckAnimation.timer = 0.0f;
        neckAnimation.startPosition = Position;
        neckAnimation.startYaw = Yaw;
        neckAnimation.startPitch = Pitch;
        neckAnimation.animationCompleted = false;

        moveForward = false;
        moveBackward = false;
        moveLeft = false;
        moveRight = false;
        isMoving = false;
        CanChangeDirection = false;
    }

    void ResetCamera() {
        Position = neckAnimation.startPosition;
        Yaw = neckAnimation.startYaw;
        Pitch = neckAnimation.startPitch;
        updateCameraVectors();
    }

private:
    bool CheckCollision(const glm::vec3& newPosition, const AABB& aabb) const {
        AABB cameraAABB;
        cameraAABB.min = newPosition - glm::vec3(0.6f, 1.0f, 0.6f);
        cameraAABB.max = newPosition + glm::vec3(0.6f, 2.0f, 0.6f);
        return !(cameraAABB.max.x < aabb.min.x || cameraAABB.min.x > aabb.max.x ||
            cameraAABB.max.y < aabb.min.y || cameraAABB.min.y > aabb.max.y ||
            cameraAABB.max.z < aabb.min.z || cameraAABB.min.z > aabb.max.z);
    }
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif
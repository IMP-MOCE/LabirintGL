#ifndef FLASHLIGHT_H
#define FLASHLIGHT_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

extern Camera camera;

class Flashlight
{
public:
	glm::vec3 Position;
	glm::vec3 Direction;
	glm::vec3 Ambient;
	glm::vec3 Diffuse;
	glm::vec3 Specular;
	float CutOff;
	float OuterCutOff;
	float Constant;
	float Linear;
	float Quadratic;
	bool State;
	float BatteryLevel;
	Flashlight(glm::vec3 position = glm::vec3(camera.Position.x - camera.Front.z*2.0f, camera.Position.y - 2.0f, camera.Position.z), glm::vec3 direction = camera.Front, glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f), float cutOff = 6.5f, float outerCutOff = 10.0f, float constant = 1.0f, float linear = 0.00027f, float quadratic = 0.000096f, bool state = false, float battery = 100.0f)
	{
		Position = position;
		Direction = direction;
		Ambient = ambient;
		Diffuse = diffuse;
		Specular = specular;
		CutOff = cutOff;
		OuterCutOff = outerCutOff;
		Constant = constant;
		Linear = linear;
		Quadratic = quadratic;
		State = state;
		BatteryLevel = battery;
	}
	void UpdateBattery(float deltaTime)
	{
		if (State && BatteryLevel > 0.0f)
		{
			BatteryLevel -= 1.5f * deltaTime;
			if (BatteryLevel <= 0.0f)
			{
				BatteryLevel = 0.0f;
				State = false;
			}
		}
	}
	void Charge()
	{
		BatteryLevel = std::min(BatteryLevel + 33.0f, 100.0f);
	}
	void TurnOn()
	{
		if (BatteryLevel > 0.0f) State = true;
	}
	void TurnOff()
	{
		State = false;
	}
	void SetIntensity(float intensity) {
		Diffuse = glm::vec3(intensity);
	}
};
#endif
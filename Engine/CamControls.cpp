#include "CamControls.h"
#include <glm/trigonometric.hpp>
#include "Transform.h"
#include "input.h"
#include "math_utility.h"
#include "Entity.h"
#include <iostream>
#include "Time.h"
#include <iomanip>

void CamControls::Update(float deltaTime)
{
	float speedMulti = 1;
	if (KeyIsDown(KMOD_LSHIFT))
		speedMulti = 4;

	if (KeyIsDown(SDLK_SPACE))
		transform->TranslateLocally(Vector3::Up * deltaTime * speedMulti);
	if (KeyIsDown(SDLK_LCTRL))
		transform->TranslateLocally(-Vector3::Up * deltaTime * speedMulti);
	if (KeyIsDown(SDLK_d))
		transform->TranslateLocally(-Vector3::Left * deltaTime * speedMulti);
	if (KeyIsDown(SDLK_a))
		transform->TranslateLocally(Vector3::Left * deltaTime * speedMulti);
	if (KeyIsDown(SDLK_w))
		transform->TranslateLocally(Vector3::Forward * deltaTime * speedMulti);
	if (KeyIsDown(SDLK_s))
		transform->TranslateLocally(-Vector3::Forward * deltaTime * speedMulti);

	glm::vec2 keyboardMouse;
	if (KeyIsDown(SDLK_UP))
		keyboardMouse.y += 1;
	if (KeyIsDown(SDLK_DOWN))
		keyboardMouse.y -= 1;
	if (KeyIsDown(SDLK_RIGHT))
		keyboardMouse.x += 1;
	if (KeyIsDown(SDLK_LEFT))
		keyboardMouse.x -= 1;

	float roll = 0;
	if (KeyIsDown(SDLK_q))
	{
		roll -= 1;
		rollUnlocked = true;
	}
	if (KeyIsDown(SDLK_e))
	{
		roll += 1;
		rollUnlocked = true;
	}

	glm::vec2 mouseMotion = GetMouseMotion();

	if(rollUnlocked)
		transform->RotateLocally(glm::radians(45 * deltaTime) * (-mouseMotion.x - keyboardMouse.x),  Vector3::Up);
	else
		transform->RotateGlobally(glm::radians(45 * deltaTime) * (-mouseMotion.x - keyboardMouse.x), Vector3::Up);
	transform->RotateLocally(glm::radians(45 * deltaTime) * (mouseMotion.y - keyboardMouse.y), Vector3::Left);
	transform->RotateLocally(glm::radians(45 * deltaTime) * roll, Vector3::Forward);

	if (KeyWentDown(SDLK_p))
	{
		std::cout << std::setw(10) << "Left: " << std::setw(16) << transform->Left() << std::setw(10) << " Pitch: " << glm::degrees(glm::pitch(transform->WorldRotation())) << '\n'
			<< std::setw(10) << "Up: " << std::setw(16) << transform->Up() << std::setw(10) << " Yaw: " << glm::degrees(glm::yaw(transform->WorldRotation())) << '\n'
			<< std::setw(10) << "Forward: " << std::setw(16) << transform->Forward() << std::setw(10) << " Roll: " << glm::degrees(glm::roll(transform->WorldRotation()))  << '\n'
			<< "Pos: " << transform->WorldPosition() << std::endl;
		ResetConsoleFormatting();
	}

	static float sroll{};
	static float syaw{};
	static float spitch{};

	if(KeyIsDown(SDLK_b))
	{
		transform->SetWorldPosition(0, 1, 0);
		transform->SetWorldRotation(spitch += keyboardMouse.y * deltaTime * Float::Pi / 4, syaw += keyboardMouse.x * deltaTime * Float::Pi / 4, sroll += roll * deltaTime * Float::Pi / 4);
	}
	else
	{
		sroll = 0;
		syaw = 0;
		spitch = 0;
	}

}

void CamControls::Initialize()
{
	transform = entity->GetComponent<Transform>();
}

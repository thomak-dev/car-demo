#include "OrbitingCam.h"
#include "Entity.h"
#include "messages.h"
#include "Transform.h"
#include "math_utility.h"
#include "input.h"

void OrbitingCam::Initialize()
{
	Component::Initialize();
	transform = entity.GetComponent<Transform>();
	target = entity.Root()->Find("Player");
}

void OrbitingCam::OnMessageReceived(Entity* origin, Message* message)
{
	HandleUpdate(message, UpdateFunctionDt{ std::bind(&OrbitingCam::Update, this, std::placeholders::_1) });
}

void OrbitingCam::Update(float deltaTime)
{
	auto targetPtr = target.lock();
	glm::vec2 mouseDelta = GetMouseMotion();
	if(targetPtr)
		center = targetPtr->GetComponent<Transform>()->WorldPosition();

	glm::vec3 offset{ 0, 0, -smoothRadius };
	//rotation = glm::rotate(rotation, mouseDelta.x * deltaTime, transform->Up());
	//rotation = glm::rotate(rotation, mouseDelta.y * deltaTime, Vector3::Left);
	
	float radiusDir = radius - smoothRadius;
	float step = deltaTime * 10;
	if (std::abs(radiusDir) <= step)
		smoothRadius = radius;
	else
		smoothRadius += step * glm::sign(radiusDir);

	radius -= GetMouseWheelDelta().y * wheelSensivity;
	radius = glm::clamp(radius, 2.f, 20.f);
	
	transform->RotateGlobally(-mouseDelta.x * deltaTime * sensivity, Vector3::Up);
	transform->RotateLocally(mouseDelta.y * deltaTime * sensivity, Vector3::Left);
	offset = transform->Rotation() * offset;
	transform->SetWorldPosition(center + offset);
	
}

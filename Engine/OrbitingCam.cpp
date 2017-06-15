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

	glm::vec3 offset{ 0, 0, -radius };
	//rotation = glm::rotate(rotation, mouseDelta.x * deltaTime, transform->Up());
	//rotation = glm::rotate(rotation, mouseDelta.y * deltaTime, Vector3::Left);
	
	
	transform->RotateGlobally(-mouseDelta.x * deltaTime, Vector3::Up);
	transform->RotateLocally(mouseDelta.y * deltaTime, Vector3::Left);
	offset = transform->Rotation() * offset;
	transform->SetWorldPosition(center + offset);
	
}

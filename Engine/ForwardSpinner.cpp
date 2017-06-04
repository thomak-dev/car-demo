#include "ForwardSpinner.h"
#include <glm/trigonometric.hpp>
#include "Transform.h"
#include "math_utility.h"
#include "Entity.h"
#include "messages.h"
#include "CamControls.h"

void ForwardSpinner::Initialize()
{
	transform = entity->GetComponent<Transform>();
}

void ForwardSpinner::OnMessageReceived(Entity* origin, Message* message)
{
	HandleUpdate(message, UpdateFunctionDt{ bind(&ForwardSpinner::Update, this, std::placeholders::_1) });
}

void ForwardSpinner::Update(float deltaTime)
{
	transform->TranslateGlobally(Vector3::Forward * deltaTime * 0.2f);
	transform->RotateLocally(glm::radians(20 * deltaTime), Vector3::Forward);
}
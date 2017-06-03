#include "ForwardSpinner.h"
#include <glm/trigonometric.hpp>
#include "Transform.h"
#include "math_utility.h"
#include "Entity.h"

void ForwardSpinner::Update(float deltaTime)
{
	transform->TranslateGlobally(Vector3::Forward * deltaTime * 0.2f);
	transform->RotateLocally(glm::radians(20 * deltaTime), Vector3::Forward);
}

void ForwardSpinner::Initialize()
{
	transform = entity->GetComponent<Transform>();
}

#pragma once
#include <memory>
#include <glm/gtc/quaternion.hpp>
#include <glm/fwd.hpp>
#include "Component.h"
#include "core.h"

class Transform;

class OrbitingCam :	public Component
{
	DELETE_COPY_MOVE(OrbitingCam)
public:
	using Component::Component;
	virtual ~OrbitingCam() = default;

	void Initialize() override;
	void OnMessageReceived(Entity* origin, Message* message) override;

private:
	std::weak_ptr<Entity> target;
	Transform* transform;
	void Update(float deltaTime);
	float radius{10};
	float smoothRadius{ radius };
	glm::quat rotation;
	glm::vec3 center;
	float sensivity{ 0.5f };
	float wheelSensivity{ 0.5f };
};


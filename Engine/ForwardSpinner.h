#pragma once
#include "core.h"
#include "Component.h"

class Transform;

class ForwardSpinner : public Component
{
	DELETE_COPY_MOVE(ForwardSpinner)
public:
	using Component::Component;
	
	void Initialize() override;
	void OnMessageReceived(Entity* origin, Message* message) override;
private:
	Transform* transform{};
	void Update(float deltaTime);
};


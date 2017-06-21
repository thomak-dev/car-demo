#pragma once
#include "Component.h"
#include "core.h"
class Updatable : public Component
{
	DELETE_COPY_MOVE(Updatable)
public:
	using Component::Component;
	virtual ~Updatable() = default;

	void OnMessageReceived(Entity* origin, Message* message) override;

protected:
	virtual void Update(float deltaTime) = 0;
};


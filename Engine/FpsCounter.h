#pragma once
#include "Component.h"
#include "core.h"

class Text;

class FpsCounter : public Component
{
	DELETE_COPY_MOVE(FpsCounter)
public:
	using Component::Component;
	virtual ~FpsCounter() = default;

	void Initialize() override;
	void OnMessageReceived(Entity* origin, Message* message) override;
	

private:
	Text* text{};
	void Update(float deltaTime);
};

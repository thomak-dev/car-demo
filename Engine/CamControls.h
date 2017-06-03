#pragma once
#include "core.h"
#include "Component.h"

class Transform;

class CamControls :	public Component
{
	DELETE_COPY_MOVE(CamControls)
public:
	CamControls() = delete;
	using Component::Component;

	virtual ~CamControls() = default;

	void Initialize() override;
	void OnMessageReceived(Entity* origin, Message* message) override;
private:
	Transform* transform{};
	bool rollUnlocked{};
	void Update(float deltaTime);
};


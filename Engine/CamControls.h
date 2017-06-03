#pragma once
#include "core.h"
#include "Component.h"
#include "Updatable.h"

class Transform;

class CamControls :	public Component, public virtual Updatable
{
	DELETE_COPY_MOVE(CamControls)
public:
	CamControls() = delete;
	using Component::Component;

	virtual ~CamControls() = default;

	void Update(float deltaTime) override;
	void Initialize() override;
private:
	Transform* transform{};
	bool rollUnlocked{};
};


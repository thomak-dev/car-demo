#pragma once
#include "core.h"
#include "Component.h"
#include "Updatable.h"

class Transform;

class ForwardSpinner : public Component, public virtual Updatable
{
	DELETE_COPY_MOVE(ForwardSpinner)
public:
	using Component::Component;

	virtual void Update(float deltaTime) override;
	virtual void Initialize() override;
private:
	Transform* transform{};
};


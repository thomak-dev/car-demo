#pragma once
#include "Component.h"
#include "core.h"
#include "Updatable.h"

class Text;

class FpsCounter : public Component, public virtual Updatable
{
	DELETE_COPY_MOVE(FpsCounter)
public:
	using Component::Component;
	virtual ~FpsCounter() = default;
	void Initialize() override;
	void Update(float deltaTime) override;

private:
	Text* text{};
};

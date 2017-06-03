#pragma once
#include "Component.h"
#include "core.h"
class RandomColor :	public Component
{
	DELETE_COPY_MOVE(RandomColor)
public:
	using Component::Component;
	virtual ~RandomColor() = default;

	void Initialize() override;
};


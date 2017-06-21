#pragma once
#include "Component.h"

class AudioListener : public Component
{
public:
	using Component::Component;
	virtual ~AudioListener();

	void Initialize() override;
};


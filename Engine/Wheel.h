#pragma once
#include "Component.h"
#include "core.h"
class Wheel : public Component
{
DELETE_COPY_MOVE(Wheel)
public:
	using Component::Component;
	virtual ~Wheel() = default;

	void Deserialize(const Json& json) override;
	int GetIndex() const { return index; }

private:
	int index{};
};


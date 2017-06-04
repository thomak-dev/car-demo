#pragma once
#include <glm/glm.hpp>

class Transform;

struct Message
{
	bool suppressed{};
	virtual ~Message() = default;
};

struct UpdateMessage : Message
{
	explicit UpdateMessage(float deltaTime) : deltaTime{deltaTime} {}
	float deltaTime{};
	virtual ~UpdateMessage() = default;
};

struct TransformChangeMessage : Message
{
	TransformChangeMessage(Transform* parent) : parent{ parent } {}
	Transform* parent{};
};

template <typename T>
void HandleUpdate(Message* message, T& target, void(T::*updateFunction)(float))
{
	UpdateMessage* update = dynamic_cast<UpdateMessage*>(message);
	if (update)
		(target.*updateFunction)(update->deltaTime);
}

template <typename T>
void HandleUpdate(Message* message, T& target, void(T::*updateFunction)())
{
	UpdateMessage* update = dynamic_cast<UpdateMessage*>(message);
	if (update)
		(target.*updateFunction)();
}
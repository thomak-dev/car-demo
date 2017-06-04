#pragma once
#include <functional>
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

using UpdateFunctionDt = std::function<void(float)>;
inline void HandleUpdate(Message* message, const UpdateFunctionDt& updateFunction)
{
	UpdateMessage* update = dynamic_cast<UpdateMessage*>(message);
	if (update)
		updateFunction(update->deltaTime);
}

using UpdateFunction = std::function<void()>;
inline void HandleUpdate(Message* message, const UpdateFunction& updateFunction)
{
	UpdateMessage* update = dynamic_cast<UpdateMessage*>(message);
	if (update)
		updateFunction();
}
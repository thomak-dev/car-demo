#pragma once

class Message
{
public:
	virtual ~Message() = default;
};

class UpdateMessage : public Message
{
public:
	explicit UpdateMessage(float deltaTime) : deltaTime{deltaTime} {}
	float deltaTime{};
	virtual ~UpdateMessage() = default;
};

template <typename T>
void HandleUpdate(Message* message, T& target, void(T::*updateFunction)(float))
{
	UpdateMessage* update = dynamic_cast<UpdateMessage*>(message);
	if (update)
		(target.*updateFunction)(update->deltaTime);
}
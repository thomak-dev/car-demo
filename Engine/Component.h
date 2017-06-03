#pragma once
#include "json.h"
#include "core.h"

class Entity;
class Message;

class Component
{
	DELETE_COPY_MOVE(Component)
public:
	using Json = rapidjson::GenericObject<true, rapidjson::Value>;

	Component() = delete;
	Component(Entity* entity);
	Entity* GetEntity() const { return entity; }
	virtual void Deserialize(const Json& json) {}
	virtual void Initialize() {}
	virtual void OnMessageReceived(Entity* origin, Message* message) {}

	virtual ~Component() = default;
protected:
	Entity* entity;
};


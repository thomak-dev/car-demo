#pragma once
#include <SDL.h>
#include "json.h"
#include "core.h"

class Entity;
struct Message;

class Component
{
	DELETE_COPY_MOVE(Component)
public:
	using Json = rapidjson::GenericObject<true, rapidjson::Value>;

	Component() = delete;
	Component(Entity& entity);
	Entity& GetEntity() const { return entity; }
	virtual int Deserialize(const Json& json) { return 0; }
	virtual void Initialize() { PRO_ASSERT(++initCount <= 1); };
	virtual void OnMessageReceived(Entity* origin, Message* message) {}

	virtual ~Component() = default;
protected:
	Entity& entity;
	int initCount{};
};


#pragma once
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <memory>
#include <vector>
#include <SDL.h>
#include "core.h"
#include "Component.h"
#include "ResourceManager.h"

namespace EntityFlags
{
	enum Pos : uint32_t
	{
		DefaultPos = 0,
		UIPos,
		ChassisPos,
		WheelPos,
		HighestPos = 31
	};
	enum Type : uint32_t
	{
		None = 0,
		Default = 1u << DefaultPos,
		UI = 1u << UIPos,
		Chassis = 1u << ChassisPos,
		Wheel = 1u << WheelPos,
		Highest = 1u << HighestPos,
		All = 0xFFFFFFFFu
	};

	Type FromString(const std::string& str);

	inline Type& operator|=(Type& first, Type second)
	{
		first = static_cast<Type>(first | second);
		return first;
	}
}

struct Message;

class Entity
{
	DELETE_COPY_MOVE(Entity)
public:
	Entity() = default;
	~Entity();

	static Entity* Instantiate(const std::shared_ptr<Prefab>& prefab, Entity* parent);
	static Entity* Instantiate(const rapidjson::GenericObject<true, rapidjson::Value>& prefab, Entity* parent);

	template <typename T>
	T* AddComponent();
	Component* AddComponent(const std::string& type);

	template <typename T>
	T* GetComponent();
	template<typename T>
	T* GetComponentInAncestors();
	Component* GetComponent(const std::string& type);
	Entity* Parent() const { return parent; }
	void SetParent(Entity* parent);
	
	const std::vector<Entity*>& Children() const { return children; };

	void Initialize() const;
	void SendMessageDown(Message* message);
	void SendMessageUp(Message* message);
	void SendMessageToSelf(Message* message);

	EntityFlags::Type flags{EntityFlags::Default};

private:
	std::unordered_map<std::type_index, Component*> components;
	std::vector<Component*> componentsInOrder;
	Entity* parent{};
	std::vector<Entity*> children;

	void AddChild(Entity* child);
	void RemoveChild(Entity* child);
	void ReceiveMessageFromAbove(Entity* origin, Message* message);
	void ReceiveMessageFromBelow(Entity* origin, Message* message);
};

template<typename T>
inline T* Entity::AddComponent()
{
	T* newComp = new T(this);
	std::type_index type{typeid(T)};
	SDL_assert(components.find(type) == components.end());
	components[type] = newComp;
	componentsInOrder.push_back(newComp);
	return newComp;
}

template<typename T>
inline T* Entity::GetComponent()
{
	auto result = components.find(std::type_index{ typeid(T) });
	if (result != components.end())
		return dynamic_cast<T*>(result->second);
	else
		return nullptr;
}

template<typename T>
T* Entity::GetComponentInAncestors()
{
	T* here = GetComponent<T>();
	if (here)
		return here;
	else if (parent)
		return parent->GetComponentInAncestors<T>();
	return here;
}


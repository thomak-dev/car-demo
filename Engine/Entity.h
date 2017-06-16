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
		PropPos,
		GroundPos,
		HighestPos = 31
	};
	enum Type : uint32_t
	{
		None = 0,
		Default = 1u << DefaultPos,
		UI = 1u << UIPos,
		Chassis = 1u << ChassisPos,
		Wheel = 1u << WheelPos,
		Prop = 1u << PropPos,
		Ground = 1u << GroundPos,
		Highest = 1u << HighestPos,
		All = 0xFFFFFFFFu
	};

	Type FromString(const std::string& str);

	inline Type operator|(Type first, Type second)
	{
		return static_cast<Type>(static_cast<std::underlying_type<Type>::type>(first) | static_cast<std::underlying_type<Type>::type>(second));
	}

	inline Type& operator|=(Type& first, Type second)
	{
		first = first | second;
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

	static std::shared_ptr<Entity> Instantiate(const std::shared_ptr<Json>& prefab, Entity* parent);
	static std::shared_ptr<Entity> Instantiate(const rapidjson::GenericObject<true, rapidjson::Value>& prefab, Entity* parent);

	template <typename T>
	T* AddComponent();
	Component* AddComponent(const std::string& type);

	template <typename T>
	T* GetComponent() const;
	template<typename T>
	T* GetComponentInAncestors();
	template<typename T, typename OutputIt>
	OutputIt GetComponentsInDescendants(OutputIt) const;
	Component* GetComponent(const std::string& type) const;
	std::string Name() const { return name; }
	void SetName(const std::string& name);
	void RenameChild(const std::string& child, const std::string& newName);
	std::shared_ptr<Entity> Parent() const { return parent.lock(); }
	std::shared_ptr<Entity> Find(const std::string& path) const;
	std::shared_ptr<Entity> GetChild(const std::string& name) const;
	void AddChild(const std::shared_ptr<Entity>& child);
	void RemoveChild(const std::shared_ptr<Entity> child);
	std::shared_ptr<Entity> CreateChild(const std::string& name);
	std::string GetValidChildName(std::string name);
	std::shared_ptr<Entity> Root() const { return root.lock(); }
	bool IsRoot() const { return root.lock() == self.lock(); }
	const std::vector<std::shared_ptr<Entity>>& Children() const { return children; }

	void Initialize();
	void SendMessageDown(Message* message);
	void SendMessageUp(Message* message);
	void SendMessageToSelf(Message* message);

	EntityFlags::Type flags{EntityFlags::Default};

private:
	std::unordered_map<std::type_index, Component*> componentsByType;
	std::vector<Component*> components;
	std::weak_ptr<Entity> parent{};
	std::vector<std::shared_ptr<Entity>> children;
	std::unordered_map<std::string, std::shared_ptr<Entity>> childrenByName;
	std::string name;
	std::weak_ptr<Entity> self;
	std::weak_ptr<Entity> root;

	int initCount{};

	void SetRoot(const std::shared_ptr<Entity> root);
	void ReceiveMessageFromAbove(Entity* origin, Message* message);
	void ReceiveMessageFromBelow(Entity* origin, Message* message);
	static void DeserializeComponents(const rapidjson::GenericObject<true, rapidjson::Value>& json, Entity* target);
	static void DeserializeChildren(const rapidjson::GenericObject<true, rapidjson::Value>& json, Entity* target);
	static void SetFlags(const rapidjson::GenericObject<true, rapidjson::Value>& json, Entity* target);
};

template<typename T>
inline T* Entity::AddComponent()
{
	T* newComp = new T(*this);
	std::type_index type{typeid(T)};
	SDL_assert(componentsByType.find(type) == componentsByType.end());
	componentsByType[type] = newComp;
	components.push_back(newComp);
	return newComp;
}

template<typename T>
inline T* Entity::GetComponent() const
{
	auto result = componentsByType.find(std::type_index{ typeid(T) });
	if (result != componentsByType.end())
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
	else
	{
		auto parentPtr = parent.lock();
		if (parentPtr)
			return parentPtr->GetComponentInAncestors<T>();
	}
	return here;
}

template <typename T, typename OutputIt>
OutputIt Entity::GetComponentsInDescendants(OutputIt iter) const
{
	T* comp = GetComponent<T>();
	if (comp)
		*iter++ = comp;
	for (const auto& child : children)
		iter = child->GetComponentsInDescendants<T>(iter);
	return iter;
}


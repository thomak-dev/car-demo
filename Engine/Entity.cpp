#include "Entity.h"
#include "Transform.h"
#include "Light.h"
#include "CamControls.h"
#include "Camera.h"
#include "ForwardSpinner.h"
#include "MeshInstance.h"
#include "Text.h"
#include "FpsCounter.h"
#include "RigidBody.h"
#include "RandomColor.h"
#include "messages.h"
#include "Vehicle.h"
#include "Wheel.h"

Entity::~Entity()
{
	for (auto it = components.rbegin(); it != components.rend(); ++it)
	{
		delete *it;
	}

	for (auto it = children.rbegin(); it != children.rend(); ++it)
	{
		delete *it;
	}
}

void Entity::DeserializeComponents(const rapidjson::GenericObject<true, rapidjson::Value>& json, Entity* target)
{
	if (json.HasMember("components"))
	{
		for (const auto& componentJson : json["components"].GetArray())
		{
			Component* component;
			if ((component = target->GetComponent(componentJson["type"].GetString())))
				component->Deserialize(componentJson.GetObject());
			else
			{
				component = target->AddComponent(componentJson["type"].GetString());
				component->Deserialize(componentJson.GetObject());
			}
		}
	}
}

void Entity::DeserializeChildren(const rapidjson::GenericObject<true, rapidjson::Value>& json, Entity* target)
{
	if (json.HasMember("children"))
	{
		for (const auto& child : json["children"].GetArray())
		{
			Instantiate(child.GetObject(), target);
		}
	}
}

void Entity::SetFlags(const rapidjson::GenericObject<true, rapidjson::Value>& json, Entity* target)
{
	if (json.HasMember("flags"))
	{
		EntityFlags::Type flags{ EntityFlags::None };
		for (const auto& elem : json["flags"].GetArray())
		{
			flags |= EntityFlags::FromString(elem.GetString());
		}
		target->flags = flags;
	}
}

Entity* Entity::Instantiate(const std::shared_ptr<Prefab>& prefab, Entity* parent)
{
	SDL_assert(prefab->IsObject());
	const Prefab* constPrefab = prefab.get();
	return Instantiate(constPrefab->GetObject(), parent);
}

Entity* Entity::Instantiate(const rapidjson::GenericObject<true, rapidjson::Value>& prefab, Entity* parent)
{
	Entity* base = nullptr;
	if (prefab.HasMember("base"))
	{
		std::string basePath{prefab["base"].GetString()};
		auto dotPos = basePath.find_last_of('.');
		if (dotPos != std::string::npos)
		{
			std::string ending{ basePath.substr(dotPos) };
			if (ending == ".prefab")
				base = Instantiate(ResourceManager::Instance().LoadPrefab(basePath), parent);
			else
				base = ResourceManager::Instance().LoadEntity(basePath, parent);
		}
		else
			base = parent->GetChild(basePath);
	}
	if (!base)
		base = new Entity;

	if (prefab.HasMember("name"))
		base->name = prefab["name"].GetString();

	SetFlags(prefab, base);
	DeserializeComponents(prefab, base);
	DeserializeChildren(prefab, base);

	base->SetParent(parent);
	return base;
}

#define ELSE_COMPONENT(T, AddOrGet) else if (type == #T) return AddOrGet##Component<T>()

Component* Entity::AddComponent(const std::string& type)
{
	if (type == "Transform")
		return AddComponent<Transform>();
	ELSE_COMPONENT(Light, Add);
	ELSE_COMPONENT(CamControls, Add);
	ELSE_COMPONENT(Camera, Add);
	ELSE_COMPONENT(ForwardSpinner, Add);
	ELSE_COMPONENT(MeshInstance, Add);
	ELSE_COMPONENT(Text, Add);
	ELSE_COMPONENT(FpsCounter, Add);
	ELSE_COMPONENT(RigidBody, Add);
	ELSE_COMPONENT(RandomColor, Add);
	ELSE_COMPONENT(Vehicle, Add);
	ELSE_COMPONENT(Wheel, Add);
	return nullptr;
}

Component* Entity::GetComponent(const std::string& type) const
{
	if (type == "Transform")
		return GetComponent<Transform>();
	ELSE_COMPONENT(Light, Get);
	ELSE_COMPONENT(CamControls, Get);
	ELSE_COMPONENT(Camera, Get);
	ELSE_COMPONENT(ForwardSpinner, Get);
	ELSE_COMPONENT(MeshInstance, Get);
	ELSE_COMPONENT(Text, Get);
	ELSE_COMPONENT(FpsCounter, Get);
	ELSE_COMPONENT(RigidBody, Get);
	ELSE_COMPONENT(RandomColor, Get);
	ELSE_COMPONENT(Vehicle, Get);
	ELSE_COMPONENT(Wheel, Get);
	return nullptr;
}

void Entity::SetName(const std::string& name)
{
	this->name = name;
}

void Entity::SetParent(Entity* parent)
{
	if (this->parent)
		this->parent->RemoveChild(this);
	this->parent = parent;
	if (parent)
		parent->AddChild(this);
}

Entity* Entity::GetChild(const std::string& name)
{
	const auto& found = childrenByName.find(name);
	if (found != childrenByName.end())
		return found->second;
	else
		return nullptr;
}

void Entity::AddChild(Entity* child)
{
	SDL_assert(child);
	auto found = childrenByName.find(child->name);
	if(found == childrenByName.end())
	{
		childrenByName.insert(found, std::make_pair(child->name, child));
		children.push_back(child);
	}
	else
	{
		std::string newName = child->name;
		auto underscore = newName.find_last_of('_');
		int number;
		if(underscore != std::string::npos && underscore < newName.size() - 1 /*&& underscore > 0*/)
		{
			std::string prefix{ newName.substr(0, underscore) };
			if (TryParseInt(newName.substr(underscore + 1), number))
			{
				int highestExisting = std::max(0, number);
				for (const auto& pair : childrenByName)
				{
					const auto& name = pair.first;
					if (name.find(prefix) == 0 && name.find_last_of('_') == prefix.size())
					{
						int existingNumber;
						if (TryParseInt(name.substr(prefix.size() + 1), existingNumber))
							highestExisting = std::max(highestExisting, existingNumber);
					}
				}
				number = highestExisting + 1;
			}
			else
				number = 1;
			newName.replace(underscore + 1, newName.size() - underscore - 1, std::to_string(number));
		}
		else
		{
			number = 1;
			if (newName.size() != 0 && newName[newName.size() - 1] != '_' || newName.size() == 0)
				newName += '_';
			newName += std::to_string(number);
		}
		child->name = newName;
		AddChild(child);
	}
}

void Entity::RemoveChild(Entity* child)
{
	if (child)
	{
		children.erase(find(children.begin(), children.end(), child));
		childrenByName.erase(child->name);
	}
}

void Entity::Initialize() const
{
	for (auto& child : children)
	{
		child->Initialize();
	}

	for (auto& component : components)
		component->Initialize();
}

void Entity::SendMessageDown(Message* message)
{
	for (Entity* child : children)
		child->ReceiveMessageFromAbove(this, message);
}

void Entity::SendMessageUp(Message* message)
{
	if (parent)
		parent->ReceiveMessageFromBelow(this, message);
}

void Entity::SendMessageToSelf(Message* message)
{
	for (Component* component : components)
		component->OnMessageReceived(this, message);
}

void Entity::ReceiveMessageFromAbove(Entity* origin, Message* message)
{
	for (Component* component : components)
		component->OnMessageReceived(origin, message);

	if(!message->suppressed)
		for (Entity* child : children)
			child->ReceiveMessageFromAbove(origin, message);
}

void Entity::ReceiveMessageFromBelow(Entity* origin, Message* message)
{
	for (Component* component : components)
		component->OnMessageReceived(origin, message);

	if (!message->suppressed && parent)
		parent->ReceiveMessageFromBelow(origin, message);
}

static std::unordered_map<std::string, EntityFlags::Type> stringToFlags
{
	{ "None", EntityFlags::Type::None },
	{ "Default", EntityFlags::Type::Default },
	{ "UI", EntityFlags::Type::UI },
	{ "Chassis", EntityFlags::Type::Chassis },
	{ "Wheel", EntityFlags::Type::Wheel },
	{ "Highest", EntityFlags::Type::Highest }
};

EntityFlags::Type EntityFlags::FromString(const std::string& str)
{
	const auto& found = stringToFlags.find(str);
	if (found != stringToFlags.end())
		return found->second;
	else
		return EntityFlags::None;
}

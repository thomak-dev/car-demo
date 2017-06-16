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
#include "OrbitingCam.h"

Entity::~Entity()
{
	for (auto it = components.rbegin(); it != components.rend(); ++it)
	{
		delete *it;
	}
}

void Entity::SetRoot(const std::shared_ptr<Entity> root)
{
	this->root = root;
	for (auto& child : children)
		child->SetRoot(root);
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

std::shared_ptr<Entity> Entity::Instantiate(const std::shared_ptr<Json>& prefab, Entity* parent)
{
	SDL_assert(prefab->IsObject());
	const Json* constPrefab = prefab.get();
	return Instantiate(constPrefab->GetObject(), parent);
}

std::shared_ptr<Entity> Entity::Instantiate(const rapidjson::GenericObject<true, rapidjson::Value>& prefab, Entity* parent)
{
	std::shared_ptr<Entity> base;
	if (prefab.HasMember("base"))
	{
		std::string basePath{prefab["base"].GetString()};
		auto dotPos = basePath.find_last_of('.');
		if (dotPos != std::string::npos)
		{
			std::string ending{ basePath.substr(dotPos) };
			if (ending == ".prefab")
				base = Instantiate(ResourceManager::Instance().LoadJson(basePath), parent);
			else
				base = ResourceManager::Instance().LoadEntity(basePath, parent);
		}
		else
			base = parent->GetChild(basePath);
	}
	if (!base)
		base = std::make_shared<Entity>();

	if (prefab.HasMember("name"))
		base->SetName(prefab["name"].GetString());

	if (!base->Parent())
	{
		if (parent)
			parent->AddChild(base);
		else
		{
			base->self = base;
			base->root = base;
		}
	}

	SetFlags(prefab, base.get());
	DeserializeComponents(prefab, base.get());
	DeserializeChildren(prefab, base.get());

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
	ELSE_COMPONENT(OrbitingCam, Add);
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
	ELSE_COMPONENT(OrbitingCam, Get);
	return nullptr;
}

void Entity::SetName(const std::string& name)
{
	auto parentPtr = parent.lock();
	if (parentPtr)
		parentPtr->RenameChild(this->name, name);
	else
		this->name = name;
}

void Entity::RenameChild(const std::string& childName, const std::string& newName)
{
	auto found = childrenByName.find(childName);
	if (found != childrenByName.end())
	{
		std::shared_ptr<Entity> child = found->second;
		child->name = GetValidChildName(newName);
		found = childrenByName.erase(found);
		childrenByName.insert(found, std::make_pair(child->name, child));
	}
}

std::shared_ptr<Entity> Entity::Find(const std::string& path) const
{
	auto delimiter = path.find_first_of('/');
	std::string name = path.substr(0, delimiter);
	if (delimiter == std::string::npos)
		return GetChild(name);
	else
		return GetChild(name)->Find(path.substr(delimiter + 1));
}

std::shared_ptr<Entity> Entity::GetChild(const std::string& name) const
{
	const auto& found = childrenByName.find(name);
	if (found != childrenByName.end())
		return found->second;
	else
		return nullptr;
}

void Entity::AddChild(const std::shared_ptr<Entity>& child)
{
	SDL_assert(child);
	auto otherParent = child->Parent();
	if (otherParent)
		otherParent->RemoveChild(child);

	child->name = GetValidChildName(child->name);
	child->parent = self;
	child->self = child;
	child->SetRoot(root.lock());
	children.push_back(child);
	childrenByName[child->name] = child;
}

void Entity::RemoveChild(const std::shared_ptr<Entity> child)
{
	if (child)
	{
		const auto& found = find(children.begin(), children.end(), child);
		if(found != children.end())
		{
			child->parent = std::weak_ptr<Entity>{};
			child->self = std::weak_ptr<Entity>{};
			childrenByName.erase(child->name);
			children.erase(found);
		}
	}
}

std::shared_ptr<Entity> Entity::CreateChild(const std::string& name)
{
	std::shared_ptr<Entity> child(new Entity);
	child->SetName(name);
	AddChild(child);
	return child;
}

std::string Entity::GetValidChildName(std::string name)
{
	if (childrenByName.find(name) == childrenByName.end())
		return name;

	auto underscore = name.find_last_of('_');
	int number;
	if (underscore != std::string::npos && underscore < name.size() - 1)
	{
		std::string prefix{ name.substr(0, underscore) };
		if (TryParseInt(name.substr(underscore + 1), number))
		{
			int highestExisting = std::max(0, number);
			for (const auto& pair : childrenByName)
			{
				const auto& childName = pair.first;
				if (childName.find(prefix) == 0 && childName.find_last_of('_') == prefix.size())
				{
					int existingNumber;
					if (TryParseInt(childName.substr(prefix.size() + 1), existingNumber))
						highestExisting = std::max(highestExisting, existingNumber);
				}
			}
			number = highestExisting + 1;
		}
		else
			number = 1;
		name.replace(underscore + 1, name.size() - underscore - 1, std::to_string(number));
	}
	else
	{
		number = 1;
		if (name.size() != 0 && name[name.size() - 1] != '_' || name.size() == 0)
			name += '_';
		name += std::to_string(number);
	}

	return GetValidChildName(name);
}

void Entity::Initialize()
{
	SDL_assert(0 == initCount++);
	for (auto& child : children)
	{
		child->Initialize();
	}

	for (auto& component : components)
		component->Initialize();
}

void Entity::SendMessageDown(Message* message)
{
	for (auto& child : children)
		child->ReceiveMessageFromAbove(this, message);
}

void Entity::SendMessageUp(Message* message)
{
	auto parentPtr = parent.lock();
	if (parentPtr)
		parentPtr->ReceiveMessageFromBelow(this, message);
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
		for (auto& child : children)
			child->ReceiveMessageFromAbove(origin, message);
}

void Entity::ReceiveMessageFromBelow(Entity* origin, Message* message)
{
	for (Component* component : components)
		component->OnMessageReceived(origin, message);

	auto parentPtr = parent.lock();
	if (!message->suppressed && parentPtr)
		parentPtr->ReceiveMessageFromBelow(origin, message);
}

static std::unordered_map<std::string, EntityFlags::Type> stringToFlags
{
	{ "None", EntityFlags::Type::None },
	{ "Default", EntityFlags::Type::Default },
	{ "UI", EntityFlags::Type::UI },
	{ "Chassis", EntityFlags::Type::Chassis },
	{ "Wheel", EntityFlags::Type::Wheel },
	{ "Prop", EntityFlags::Type::Prop },
	{ "Ground", EntityFlags::Type::Ground },
	{ "Highest", EntityFlags::Type::Highest },
	{ "All", EntityFlags::Type::All }
};

EntityFlags::Type EntityFlags::FromString(const std::string& str)
{
	const auto& found = stringToFlags.find(str);
	if (found != stringToFlags.end())
		return found->second;
	else
		return EntityFlags::None;
}

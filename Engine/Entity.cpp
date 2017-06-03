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

Entity::~Entity()
{
	for (auto it = componentsInOrder.rbegin(); it != componentsInOrder.rend(); ++it)
	{
		delete *it;
	}

	for (auto it = children.rbegin(); it != children.rend(); ++it)
	{
		delete *it;
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
		std::string ending{basePath.substr(basePath.find_last_of('.'))};
		if (ending == ".prefab")
			base = Instantiate(ResourceManager::Instance().LoadPrefab(basePath), parent);
		else
			base = ResourceManager::Instance().LoadEntity(basePath, parent);
	}
	if (!base)
		base = new Entity;
	if(prefab.HasMember("flags"))
	{
		EntityFlags::Type flags{ EntityFlags::None };
		for (const auto& elem : prefab["flags"].GetArray())
		{
			flags |= EntityFlags::FromString(elem.GetString());
		}
		base->flags = flags;
	}
	if (prefab.HasMember("components"))
	{
		for (const auto& componentJson : prefab["components"].GetArray())
		{
			Component* component = nullptr;
			if ((component = base->GetComponent(componentJson["type"].GetString())))
				component->Deserialize(componentJson.GetObject());
			else
			{
				component = base->AddComponent(componentJson["type"].GetString());
				component->Deserialize(componentJson.GetObject());
			}
		}
	}
	if (prefab.HasMember("children"))
	{
		for (const auto& child : prefab["children"].GetArray())
		{
			Instantiate(child.GetObject(), base);
		}
	}

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
	return nullptr;
}

Component* Entity::GetComponent(const std::string& type)
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
	return nullptr;
}

void Entity::SetParent(Entity* parent)
{
	if (this->parent)
		this->parent->RemoveChild(this);
	this->parent = parent;
	if (parent)
		parent->AddChild(this);
}

void Entity::AddChild(Entity* child)
{
	SDL_assert(child);
	children.push_back(child);
}

void Entity::RemoveChild(Entity* child)
{
	if (child)
		children.erase(find(children.begin(), children.end(), child));
}

void Entity::Initialize() const
{
	for (auto& child : children)
	{
		child->Initialize();
	}

	for (auto& component : componentsInOrder)
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
	for (Component* component : componentsInOrder)
		component->OnMessageReceived(this, message);
}

void Entity::ReceiveMessageFromAbove(Entity* origin, Message* message)
{
	for (Component* component : componentsInOrder)
		component->OnMessageReceived(origin, message);

	for (Entity* child : children)
		ReceiveMessageFromAbove(origin, message);
}

void Entity::ReceiveMessageFromBelow(Entity* origin, Message* message)
{
	for (Component* component : componentsInOrder)
		component->OnMessageReceived(origin, message);

	if (parent)
		parent->ReceiveMessageFromBelow(origin, message);
}

static std::unordered_map<std::string, EntityFlags::Type> stringToFlags
{
	{"None", EntityFlags::Type::None},
	{"Default", EntityFlags::Type::Default},
	{"UI", EntityFlags::Type::UI},
	{"Highest", EntityFlags::Type::Highest}
};

EntityFlags::Type EntityFlags::FromString(const std::string& str)
{
	const auto& found = stringToFlags.find(str);
	if (found != stringToFlags.end())
		return found->second;
	else
		return EntityFlags::None;
}

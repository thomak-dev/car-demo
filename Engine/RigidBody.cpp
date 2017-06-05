#include "RigidBody.h"
#include "Physics.h"
#include "Transform.h"
#include "math_utility.h"
#include "Entity.h"
#include "messages.h"

std::unordered_map<std::string, RigidBody::Shape> RigidBody::stringToShape
{
	{"Box", Shape::Box},
	{"Sphere", Shape::Sphere },
	{"Mesh", Shape::Mesh }
};

RigidBody::RigidBody(Entity* entity)
	:Component{entity}
{
}


RigidBody::~RigidBody()
{
	Physics::Instance().UnregisterRigidBody(this);
}

void RigidBody::Deserialize(const Json& json)
{
	if (json.HasMember("half_size"))
		halfSize = Vec3FromJson(json["half_size"]);
	if (json.HasMember("mesh"))
		mesh = ResourceManager::Instance().LoadMesh(json["mesh"].GetString());
	if (json.HasMember("shape"))
		shapeType = stringToShape[json["shape"].GetString()];
	if (json.HasMember("mass"))
		mass = json["mass"].GetFloat();
}

void RigidBody::Initialize()
{
	transform = entity->GetComponent<Transform>();
	Physics::Instance().RegisterRigidBody(this);
}

void RigidBody::OnMessageReceived(Entity* origin, Message* message)
{
	HandleUpdate(message, UpdateFunction{ std::bind(&RigidBody::Update, this) });
}

void RigidBody::SetShape(Shape shapeType)
{

}

void RigidBody::SetMass(float mass)
{
	this->mass = mass;
}

void RigidBody::Update()
{

}

void RigidBody::PostProcessPhysics()
{

}

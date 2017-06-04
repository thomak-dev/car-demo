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
	delete bulletRigidBody;
	delete shape;
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
	shape = GenerateShape(shapeType);
	btVector3 inertia{ 0, 0, 0 };
	shape->setLocalScaling(GlmToBtVec3(transform->WorldScale()));
	if (shapeType == Shape::Mesh)
	{
		//dynamic_cast<btBvhTriangleMeshShape*>(shape)->setLocalScaling();
		SDL_assert(mass == 0);
	}
	else if(mass > 0)
		shape->calculateLocalInertia(mass, inertia);

	btRigidBody::btRigidBodyConstructionInfo rbCtorInfo{ mass, this, shape, inertia };
	bulletRigidBody = new btRigidBody(rbCtorInfo);
	Physics::Instance().RegisterRigidBody(this);
}

void RigidBody::OnMessageReceived(Entity* origin, Message* message)
{
	HandleUpdate(message, *this, &RigidBody::Update);
}

void RigidBody::getWorldTransform(btTransform& worldTrans) const
{
	glm::vec3 position{ transform->WorldPosition() };
	worldTrans.setOrigin(btVector3{ position.x, position.y, position.z });
	glm::quat rotation{ transform->WorldRotation() };
	worldTrans.setRotation(btQuaternion{ rotation.x, rotation.y, rotation.z, rotation.w });
}

void RigidBody::setWorldTransform(const btTransform& worldTrans)
{
	glm::vec3 position{ glm::uninitialize };
	BtToGlmVec3(position, worldTrans.getOrigin());
	glm::quat newRotation{ glm::uninitialize };
	BtToGlmQuat(newRotation, worldTrans.getRotation());
	lastPhysicsPosition = position;
	lastPhysicsRotation = newRotation;
	//transform->SetWorldPosition(position);
	//transform->SetWorldRotation(newRotation);
}

void RigidBody::SetShape(Shape shapeType)
{
	if (shapeType == Shape::Mesh)
		SDL_assert(mass == 0);
	btCollisionShape* newShape{ GenerateShape(shapeType) };
	bulletRigidBody->setCollisionShape(newShape);
	btVector3 inertia{0, 0, 0};
	if(mass > 0)
		newShape->calculateLocalInertia(mass, inertia);
	bulletRigidBody->setMassProps(mass, inertia);
	delete shape;
	shape = newShape;
}

void RigidBody::SetMass(float mass)
{
	if (shapeType == Shape::Mesh)
		SDL_assert(mass == 0);
	if(this->mass != mass)
	{
		this->mass = mass;
		btVector3 inertia{0, 0, 0};
		if(mass > 0)
			shape->calculateLocalInertia(mass, inertia);
		bulletRigidBody->setMassProps(mass, inertia);
	}
}

void RigidBody::Update()
{
	if(mass > 0)
	{
		transform->SetWorldPosition(lastPhysicsPosition);
		transform->SetWorldRotation(lastPhysicsRotation);
	}
}

btCollisionShape* RigidBody::GenerateShape(Shape type) const
{
	switch (shapeType)
	{
	case Shape::Box:
		return new btBoxShape{ GlmToBtVec3(halfSize) };
	case Shape::Sphere:
		return new btSphereShape{ halfSize.x };
	case Shape::Mesh:
		return new btBvhTriangleMeshShape{ mesh.get(), true };
	default:
		SDL_assert(false);
		return nullptr;
	}
}

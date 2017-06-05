#include "RigidBody.h"
#include "Physics.h"
#include "Transform.h"
#include "math_utility.h"
#include "Entity.h"
#include "messages.h"

using namespace physx;

Physics* RigidBody::physics{nullptr};

std::unordered_map<std::string, RigidBody::Shape> RigidBody::stringToShape
{
	{"Box", Shape::Box},
	{"Sphere", Shape::Sphere},
	{"Mesh", Shape::Mesh}
};

RigidBody::RigidBody(Entity* entity)
	: Component{entity}
{
}


RigidBody::~RigidBody()
{
	physics->UnregisterRigidBody(this);
	if (rigidActor)
		rigidActor->release();
	if (shape)
		shape->release();
}

void RigidBody::Deserialize(const Json& json)
{
	if (json.HasMember("half_size"))
		halfSize = Vec3FromJson(json["half_size"]);
	if (json.HasMember("mesh"))
		mesh = ResourceManager::Instance().LoadMesh(json["mesh"].GetString());
	if (json.HasMember("shape"))
		shapeType = stringToShape[json["shape"].GetString()];
	if (json.HasMember("density"))
		density = json["density"].GetFloat();
	if (json.HasMember("static"))
		isStatic = json["static"].GetBool();
}

void RigidBody::Initialize()
{
	transform = entity->GetComponent<Transform>();
	PxTransform pTransform{ToPxMat44(transform->WorldMatrix())};
	if (isStatic)
		rigidActor = physics->backend->createRigidStatic(pTransform.getNormalized());
	else
	{
		PxRigidDynamic* dyn = physics->backend->createRigidDynamic(pTransform.getNormalized());
		rigidActor = dyn;
	}
	SetShapeInternal(shapeType);
	SetDensityInternal(density);

	physics->RegisterRigidBody(this);
}

void RigidBody::OnMessageReceived(Entity* origin, Message* message)
{
	HandleUpdate(message, UpdateFunction{std::bind(&RigidBody::Update, this)});
	UpdateMessage* update = dynamic_cast<UpdateMessage*>(message);
	if (update && update->isFixed)
		PostProcessPhysics();
}

void RigidBody::SetShape(Shape shape)
{
	if (this->shapeType != shape)
		SetShapeInternal(shape);
}


void RigidBody::SetDensity(float density)
{
	if (this->density != density)
		SetDensityInternal(density);
}

void RigidBody::Update()
{
	transform->SetWorldPosition(lastPhysicsPosition);
	transform->SetWorldRotation(lastPhysicsRotation);
}

void RigidBody::PostProcessPhysics()
{
	auto pTrans = rigidActor->getGlobalPose();
	lastPhysicsRotation = ToGlmQuat(pTrans.q);
	lastPhysicsPosition = ToGlmVec3(pTrans.p);
	transform->SetWorldPosition(lastPhysicsPosition);
	transform->SetWorldRotation(lastPhysicsRotation);
}

void RigidBody::SetShapeInternal(Shape shapeType)
{
	this->shapeType = shapeType;
	if (shape)
	{
		rigidActor->detachShape(*shape);
		shape->release();
	}

	switch (shapeType)
	{
	case Shape::Box:
		shape = physics->backend->createShape(PxBoxGeometry{ToPxVec3(halfSize * transform->WorldScale())}, *physics->defaultMaterial, true);
		rigidActor->attachShape(*shape);
		break;
	case Shape::Sphere:
		shape = physics->backend->createShape(PxSphereGeometry{halfSize.x * transform->WorldScale().x}, *physics->defaultMaterial, true);
		rigidActor->attachShape(*shape);
		break;
	case Shape::Mesh:
		SDL_assert(isStatic);
		shape = physics->backend->createShape(PxTriangleMeshGeometry{physics->GetMesh(mesh), PxMeshScale{ToPxVec3(transform->WorldScale())}}, *physics->defaultMaterial, true);
		rigidActor->attachShape(*shape);
		break;
	default: ;
	}
}

void RigidBody::SetDensityInternal(float density)
{
	this->density = density;
	PxRigidDynamic* dynamic = rigidActor->is<PxRigidDynamic>();
	if (dynamic)
		PxRigidBodyExt::updateMassAndInertia(*dynamic, &density, 1);
}

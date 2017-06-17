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
	{"Mesh", Shape::Mesh},
	{"Terrain", Shape::Terrain},
	{"Convex", Shape::Convex}
};

RigidBody::RigidBody(Entity& entity)
	: Component{entity}
{
	physics->SetUpFilterData(filterData, entity.flags);
}


RigidBody::~RigidBody()
{
	physics->UnregisterRigidBody(this);
	if (rigidActor)
		rigidActor->release();
	if (shape)
		shape->release();
}

int RigidBody::Deserialize(const Json& json)
{
	int count = 0;
	if (json.HasMember("half_size") && ++count)
		halfSize = ToVec3(json["half_size"]);
	if (json.HasMember("mesh") && ++count)
		mesh = ResourceManager::Instance().LoadMesh(json["mesh"].GetString());
	if (json.HasMember("shape") && ++count)
		shapeType = stringToShape[json["shape"].GetString()];
	if (json.HasMember("density") && ++count)
		density = json["density"].GetFloat();
	if (json.HasMember("static") && ++count)
		isStatic = json["static"].GetBool();
	if (json.HasMember("offset") && ++count)
		offset = ToVec3(json["offset"]);
	if (json.HasMember("material") && ++count)
		material = physics->materials[json["material"].GetString()];
	if (!material)
		material = physics->materials["Default"];

	return count;
}

void RigidBody::Initialize()
{
	Component::Initialize();
	transform = entity.GetComponent<Transform>();
	PxTransform pTransform{ToPxMat44(transform->WorldMatrix())};
	if (isStatic)
		rigidActor = physics->backend->createRigidStatic(pTransform.getNormalized());
	else
	{
		PxRigidDynamic* dyn = physics->backend->createRigidDynamic(pTransform.getNormalized());
		rigidActor = dyn;
	}
	
	InitCommonProps();
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
	{
		this->density = density;
		UpdateMassAndInertia();
	}
}

void RigidBody::UpdateTransform()
{
	auto pTrans = rigidActor->getGlobalPose();
	lastPhysicsRotation = ToQuat(pTrans.q);
	lastPhysicsPosition = ToVec3(pTrans.p);
	transform->SetWorldPosition(lastPhysicsPosition);
	transform->SetWorldRotation(lastPhysicsRotation);
}

void RigidBody::Update()
{
	transform->SetWorldPosition(lastPhysicsPosition);
	transform->SetWorldRotation(lastPhysicsRotation);
}

void RigidBody::InitCommonProps()
{
	SetShapeInternal(shapeType);
	UpdateMassAndInertia();
	rigidActor->userData = this;
	lastPhysicsPosition = transform->WorldPosition();
	lastPhysicsRotation = transform->WorldRotation();
}

void RigidBody::PostProcessPhysics()
{

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
		shape = physics->backend->createShape(PxBoxGeometry{ToPxVec3(halfSize * transform->WorldScale())}, *material, true);
		shape->setLocalPose(PxTransform(ToPxVec3(offset * transform->WorldScale())));
		rigidActor->attachShape(*shape);
		break;
	case Shape::Sphere:
		shape = physics->backend->createShape(PxSphereGeometry{halfSize.x * transform->WorldScale().x}, *material, true);
		shape->setLocalPose(PxTransform(ToPxVec3(offset * transform->WorldScale())));
		rigidActor->attachShape(*shape);
		break;
	case Shape::Mesh:
		SDL_assert(isStatic);
		shape = physics->backend->createShape(PxTriangleMeshGeometry{physics->GetMesh(mesh), PxMeshScale{ToPxVec3(transform->WorldScale())}}, *material, true);
		shape->setLocalPose(PxTransform(ToPxVec3(offset * transform->WorldScale())));
		rigidActor->attachShape(*shape);
		break;
	case Shape::Terrain:
		{
			SDL_assert(isStatic);
			const Physics::Terrain& terrain = *physics->GetTerrain(mesh);
			shape = physics->backend->createShape(PxHeightFieldGeometry(terrain.heightField, PxMeshGeometryFlags{}, terrain.height / (1 << 16), 1, 1), *material, true);
			PxTransform pose{PxVec3{terrain.minX, terrain.minHeight + terrain.height / 2, terrain.minZ} + ToPxVec3(offset * transform->WorldScale()) };
			shape->setLocalPose(pose);
			rigidActor->attachShape(*shape);
			break;
		}
	case Shape::Convex:
		shape = physics->backend->createShape(PxConvexMeshGeometry{ physics->GetConvexMesh(mesh), PxMeshScale{ ToPxVec3(transform->WorldScale()) } }, *material, true);
		shape->setLocalPose(PxTransform(ToPxVec3(offset * transform->WorldScale())));
		rigidActor->attachShape(*shape);
		break;
	default: SDL_assert(false);
	}

	shape->setQueryFilterData(filterData);
	shape->setSimulationFilterData(filterData);
}

void RigidBody::UpdateMassAndInertia()
{
	PxRigidDynamic* dynamic = rigidActor->is<PxRigidDynamic>();
	if (dynamic)
		PxRigidBodyExt::updateMassAndInertia(*dynamic, &density, 1);
}

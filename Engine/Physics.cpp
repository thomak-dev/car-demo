#include "Physics.h"
#include <iostream>
#include "RigidBody.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Mesh.h"

using namespace physx;

void Physics::PhysxErrorCallback::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	std::cout << "Physx reports ";
	switch (code)
	{
	case PxErrorCode::eNO_ERROR:
		std::cout << "no error";
		break;
	case PxErrorCode::eINVALID_PARAMETER:
		std::cout << "invalid parameter";
		break;
	case PxErrorCode::eINVALID_OPERATION:
		std::cout << "invalid operation";
		break;
	case PxErrorCode::eOUT_OF_MEMORY:
		std::cout << "out of memory";
		break;
	case PxErrorCode::eDEBUG_INFO:
		std::cout << "info";
		break;
	case PxErrorCode::eDEBUG_WARNING:
		std::cout << "warning";
		break;
	case PxErrorCode::ePERF_WARNING:
		std::cout << "performance warning";
		break;
	case PxErrorCode::eABORT:
		std::cout << "abort";
		break;
	case PxErrorCode::eINTERNAL_ERROR:
		std::cout << "internal error";
		break;
	case PxErrorCode::eMASK_ALL:
		std::cout << "unknown error";
		break;
	}

	std::cout << ": " << message << "\nFile: " << file << '(' << line << ')' << std::endl;
}

Physics::Physics()
{	
	RigidBody::physics = this;
	foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, allocator, errorCallback);
	SDL_assert(foundation);

	pvdTransport = PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
	pvd = PxCreatePvd(*foundation);
	pvd->connect(*pvdTransport, PxPvdInstrumentationFlag::eALL);

	PxTolerancesScale tolerance{};
	tolerance.speed = 9.81f;
	backend = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, tolerance, false, pvd);
	cooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, PxCookingParams(tolerance));

	SDL_assert(backend && cooking && PxInitExtensions(*backend, pvd));
	
	defaultMaterial = backend->createMaterial(0.5f, 0.5f, 0.5f);
	PxSceneDesc scnDesc{ tolerance };
	cpuDispatcher = PxDefaultCpuDispatcherCreate(4);
	scnDesc.cpuDispatcher = cpuDispatcher;
	scnDesc.filterShader = PxDefaultSimulationFilterShader;
	scnDesc.gravity = PxVec3{ 0, -9.81f, 0 };
	scene = backend->createScene(scnDesc);	
	scene->setFlag(PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);
	scene->setFlag(PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS, true);
}


Physics::~Physics()
{
	for(auto& pair : physicsMeshes)
	{
		pair.second->release();
	}
	scene->release();
	defaultMaterial->release();
	PxCloseExtensions();
	cooking->release();
	backend->release();
	pvd->release();
	pvdTransport->release();
	foundation->release();
}

void Physics::RegisterRigidBody(RigidBody* rigidBody)
{
	rigidBodies.push_back(rigidBody);
	scene->addActor(*rigidBody->rigidActor);
}

void Physics::UnregisterRigidBody(RigidBody* rigidBody)
{
	auto found = std::find(rigidBodies.begin(), rigidBodies.end(), rigidBody);
	if(found != rigidBodies.end())
	{
		scene->removeActor(*rigidBody->rigidActor);
		rigidBodies.erase(found);
	}
}

void Physics::Step(float deltaTime)
{
	scene->simulate(deltaTime);
}

void Physics::Await()
{
	scene->fetchResults(true);
	PxU32 activeActorsCount;
	PxActor** activeActors = scene->getActiveActors(activeActorsCount);
	for (int i = 0; i < activeActorsCount; ++i)
	{
		RigidBody* rigidBody = static_cast<RigidBody*>(activeActors[i]->userData);
		SDL_assert(rigidBody);
		rigidBody->UpdateTransform();
	}
}

void Physics::DebugDraw()
{

}

PxTriangleMesh* Physics::GetMesh(const std::shared_ptr<Mesh>& mesh)
{
	auto found = physicsMeshes.find(mesh);
	if(found == physicsMeshes.end())
	{
		PxCookingParams params{ backend->getTolerancesScale() };
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
		params.meshCookingHint = PxMeshCookingHint::eCOOKING_PERFORMANCE;
		cooking->setParams(params);

		PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = mesh->vertices.size();
		meshDesc.points.stride = sizeof(decltype(mesh->vertices)::value_type);
		meshDesc.points.data = mesh->vertices.data();
		meshDesc.triangles.count = mesh->indices.size() / 3;
		meshDesc.triangles.stride = 3 * sizeof(decltype(mesh->indices)::value_type);
		meshDesc.triangles.data = mesh->indices.data();

		PxTriangleMesh* physicsMesh = cooking->createTriangleMesh(meshDesc, backend->getPhysicsInsertionCallback());
		physicsMeshes.insert(found, make_pair(mesh, physicsMesh));
		return physicsMesh;
	}

	return found->second;
}
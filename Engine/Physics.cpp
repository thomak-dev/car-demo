#include "Physics.h"
#include <iostream>
#include "RigidBody.h"
#include "Renderer.h"

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
	foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, allocator, errorCallback);
	SDL_assert(foundation);

	pvd = PxCreatePvd(*foundation);
	pvdTransport = PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
	pvd->connect(*pvdTransport, PxPvdInstrumentationFlag::eALL);

	backend = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation,
		PxTolerancesScale{}, false, pvd);
	
	SDL_assert(backend && PxInitExtensions(*backend, pvd));
}


Physics::~Physics()
{
	PxCloseExtensions();
	backend->release();
	pvdTransport->release();
	pvd->release();
}

void Physics::RegisterRigidBody(RigidBody* rigidBody)
{
	rigidBodies.push_back(rigidBody);
}

void Physics::UnregisterRigidBody(RigidBody* rigidBody)
{
	auto found = std::find(rigidBodies.begin(), rigidBodies.end(), rigidBody);
	if(found != rigidBodies.end())
	{
		rigidBodies.erase(found);
	}
}

void Physics::Step(float deltaTime)
{
	for(auto rigidBody : rigidBodies)
	{
		
	}
}

void Physics::DebugDraw()
{

}
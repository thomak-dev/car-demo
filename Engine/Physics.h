#pragma once
#include <vector>
#include <PxPhysicsAPI.h>
#include "Singleton.h"

class RigidBody;

class Physics : public Singleton<Physics>
{
public:
	Physics();
	~Physics();

	void RegisterRigidBody(RigidBody* rigidBody);
	void UnregisterRigidBody(RigidBody* rigidBody);
	void Step(float deltaTime);
	void DebugDraw();

	bool Visualize() const { return visualize; }
	void SetVisualize(bool visualize) { this->visualize = visualize; }

private:
	class PhysxErrorCallback : public physx::PxErrorCallback
	{
	public:
		virtual ~PhysxErrorCallback() = default;
		void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
	};

	bool visualize{};
	physx::PxFoundation* foundation;
	physx::PxPvd* pvd;
	physx::PxPvdTransport* pvdTransport;
	physx::PxDefaultAllocator allocator;
	PhysxErrorCallback errorCallback;
	physx::PxPhysics* backend;

	std::vector<RigidBody*> rigidBodies;
};


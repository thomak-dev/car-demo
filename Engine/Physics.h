#pragma once
#include <vector>
#include <PxPhysicsAPI.h>
#include <memory>
#include <unordered_map>
#include "Singleton.h"

class Mesh;
class RigidBody;

class Physics : public Singleton<Physics>
{
	friend class RigidBody;
public:
	struct Terrain
	{
		float height;
		float minX;
		float minZ;
		float minHeight;
		physx::PxHeightField* heightField;
	};

	Physics();
	~Physics();

	void RegisterRigidBody(RigidBody* rigidBody);
	void UnregisterRigidBody(RigidBody* rigidBody);
	void Step(float deltaTime);
	void Await();
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
	physx::PxMaterial* defaultMaterial;
	physx::PxScene* scene;
	physx::PxCpuDispatcher* cpuDispatcher;
	physx::PxCooking* cooking;

	std::vector<RigidBody*> rigidBodies;
	std::unordered_map<std::shared_ptr<Mesh>, physx::PxTriangleMesh*> physicsMeshes;
	std::unordered_map<std::shared_ptr<Mesh>, Terrain*> terrains;

	physx::PxTriangleMesh* GetMesh(const std::shared_ptr<Mesh>& mesh);
	const Terrain* GetTerrain(const std::shared_ptr<Mesh>& mesh);
};


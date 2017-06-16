#pragma once
#include <vector>
#include <PxPhysicsAPI.h>
#include <memory>
#include <unordered_map>
#include "Singleton.h"
#include "Entity.h"

class Mesh;
class RigidBody;

class Physics : public Singleton<Physics>
{
	friend class RigidBody;
	friend class Vehicle;
public:
	struct Terrain
	{
		float height;
		float minX;
		float minZ;
		float minHeight;
		physx::PxHeightField* heightField;
	};

	Physics(int maxVehicles = 64);
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

	const int MaxVehicles;
	const int MaxWheels;

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
	physx::PxRaycastQueryResult* vehQueryResults;
	physx::PxRaycastHit* vehQueryHitBuffer;
	physx::PxBatchQuery* vehicleQuery;
	physx::PxVehicleWheelQueryResult* vehWheelQueryResultBuffer;

	std::vector<RigidBody*> rigidBodies;
	std::vector<physx::PxVehicleWheels*> wheels;
	std::vector<Vehicle*> vehicles;
	physx::PxVehicleDrivableSurfaceToTireFrictionPairs* surfaceToFriction;
	
	std::unordered_map<std::shared_ptr<Mesh>, physx::PxTriangleMesh*> physicsMeshes;
	std::unordered_map<std::shared_ptr<Mesh>, Terrain*> terrains;
	EntityFlags::Type wantedCollisionsOf[EntityFlags::HighestPos]{};

	physx::PxTriangleMesh* GetMesh(const std::shared_ptr<Mesh>& mesh);
	const Terrain* GetTerrain(const std::shared_ptr<Mesh>& mesh);
	void SetUpFilterData(physx::PxFilterData& filterData, EntityFlags::Type entityType);
};


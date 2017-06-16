#pragma once
#include "RigidBody.h"
#include "math_utility.h"

namespace Tire
{
	enum Type: physx::PxU32
	{
		Normal, Slick, Winter, Spikes, Highest
	};

	Type FromString(const std::string& str);
}

class Vehicle :	public RigidBody
{
	friend class Physics;
public:
	explicit Vehicle(Entity& entity);
	virtual ~Vehicle();

	void Deserialize(const Json& json) override;
	void Initialize() override;

protected:
	void Update() override;
	void UpdateTransform() override;
	void UpdateMassAndInertia() override;

private:
	const int NumWheels;
	physx::PxVehicleDrive4W* wheels{};
	float wheelMass{ 10 };
	float wheelRadius{ .5f };
	float wheelWidth{ .3f };
	float steer{ Float::Pi / 4 };
	Tire::Type tireType{ Tire::Normal };
	std::vector<Transform*> wheelTransforms;
	std::vector<physx::PxTransform> lastKnownWheelTransforms;
	bool airborne{};
	physx::PxVehicleDrive4WRawInputData vehicleInputData;
	bool forward{ true };
	physx::PxRigidDynamic* rigidDynamic{};

	void PostProcessPhysics() override;
	void BeforeVehicleUpdate(float deltaTime);
	void AfterVehicleUpdate(const physx::PxVehicleWheelQueryResult& queryResult);
	static physx::PxConvexMesh* CreateWheelMesh(float radius, float width);

	void ConfigureDifferential(physx::PxVehicleDriveSimData4W& simData);
	void ConfigureEngine(physx::PxVehicleDriveSimData4W& simData);
	void ConfigureGears(physx::PxVehicleDriveSimData4W& simData);
	void ConfigureClutch(physx::PxVehicleDriveSimData4W& simData);
	void ConfigureAckermannCorrection(physx::PxVehicleDriveSimData4W& simData, const physx::PxVehicleWheelsSimData& wheelsSimData);
	void SetUpEachWheel(physx::PxVehicleWheelsSimData& simData, int numWheels);
	void SetWheelShapes(int numWheels);

};


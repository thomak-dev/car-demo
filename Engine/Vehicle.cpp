#include "Vehicle.h"
#include <vehicle/PxVehicleSDK.h>
#include <PxTkStream.h>
#include "Entity.h"
#include "Transform.h"
#include "math_utility.h"
#include "Physics.h"

using namespace physx;

Vehicle::~Vehicle()
{
	wheels->free();
}

void Vehicle::Initialize()
{
	Component::Initialize();
	transform = entity->GetComponent<Transform>();
	PxTransform pTrans{ ToPxMat44(transform->WorldMatrix()) };
	PxRigidDynamic* dynamicActor = physics->backend->createRigidDynamic(pTrans.getNormalized());
	rigidActor = dynamicActor;
	shape = physics->backend->createShape(PxBoxGeometry{ 1, 0.5f, 1.5f }, *physics->defaultMaterial, true);
	filterData.word0 |= EntityFlags::Chassis;
	shape->setSimulationFilterData(filterData);
	shape->setQueryFilterData(filterData);
	
	dynamicActor->attachShape(*shape);

	SetDensityInternal(density);

	const PxU32 numWheels = 4;
	PxVec3 wheelOffsets[numWheels];
	int wheelsFound = 0;
	const auto& children = entity->Children();
	for (int i = 0; i < children.size(); ++i)
	{
		if (children[i]->flags & EntityFlags::Wheel)
			wheelOffsets[wheelsFound++] = ToPxVec3(children[i]->GetComponent<Transform>()->Position());		
	}
	SDL_assert(wheelsFound >= numWheels);

	PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(numWheels);
	PxVehicleWheelData wheelsData[numWheels];
	PxVehicleTireData tires[numWheels];
	PxVehicleSuspensionData suspensions[numWheels];
	PxVec3 suspTravelDirections[numWheels];
	PxVec3 wheelCentreCMOffsets[numWheels];
	PxVec3 suspForceAppCMOffsets[numWheels];
	PxVec3 tireForceAppCMOffsets[numWheels];

	PxF32 suspSprungMasses[numWheels];
	PxVehicleComputeSprungMasses(numWheels, wheelOffsets, dynamicActor->getCMassLocalPose().p, dynamicActor->getMass(), 1, suspSprungMasses);

	PxFilterData qryFilterData;
	qryFilterData.word1 = EntityFlags::Default;

	for (int i = 0; i < numWheels; ++i)
	{
		wheelsData[i].mMOI = 0.5f * wheelMass * wheelRadius * wheelRadius;
		wheelsData[i].mMass = wheelMass;
		wheelsData[i].mWidth = wheelWidth;
		wheelsData[i].mRadius = wheelRadius;

		tires[i].mType = type;

		suspensions[i].mMaxCompression = 0.3f;
		suspensions[i].mMaxDroop = 0.1f;
		suspensions[i].mSpringStrength = 35000.0f;
		suspensions[i].mSpringDamperRate = 4500.0f;
		suspensions[i].mSprungMass = suspSprungMasses[i];

		const PxF32 camberAngleAtRest = 0.0;
		const PxF32 camberAngleAtMaxDroop = 0.01f;
		const PxF32 camberAngleAtMaxCompression = -0.01f;

		suspensions[i + 0].mCamberAtRest = camberAngleAtRest * (1 - (i % 2) * 2);
		suspensions[i + 0].mCamberAtMaxDroop = camberAngleAtMaxDroop * (1 - (i % 2) * 2);
		suspensions[i + 0].mCamberAtMaxCompression = camberAngleAtMaxCompression * (1 - (i % 2) * 2);

		suspTravelDirections[i] = PxVec3(0, -1, 0);

		wheelCentreCMOffsets[i] = wheelOffsets[i] - dynamicActor->getCMassLocalPose().p;

		//Suspension force application point 0.3 metres below rigid body center of mass.
		suspForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);

		//Tire force application point 0.3 metres below rigid body center of mass.
		tireForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);

		wheelsSimData->setWheelData(i, wheelsData[i]);
		wheelsSimData->setTireData(i, tires[i]);
		wheelsSimData->setSuspensionData(i, suspensions[i]);
		wheelsSimData->setSuspTravelDirection(i, suspTravelDirections[i]);
		wheelsSimData->setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
		wheelsSimData->setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
		wheelsSimData->setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
		wheelsSimData->setSceneQueryFilterData(i, qryFilterData);
		wheelsSimData->setWheelShapeMapping(i, i);
	}

	wheelsData[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = steer;
	wheelsData[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = steer;
	wheelsData[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque = 3000.f;
	wheelsData[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = 3000.f;
	

	PxVehicleDriveSimData4W driveSimData;
	PxVehicleDifferential4WData diff;
	diff.mType = PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_FRONTWD;
	driveSimData.setDiffData(diff);

	PxVehicleEngineData engine;
	engine.mPeakTorque = 500.0f;
	engine.mMaxOmega = 600.0f; //approx 6000 rpm
	driveSimData.setEngineData(engine);

	PxVehicleGearsData gears;
	gears.mSwitchTime = 0.5f;
	driveSimData.setGearsData(gears);
	

	PxVehicleClutchData clutch;
	clutch.mStrength = 10.0f;
	driveSimData.setClutchData(clutch);

	PxVehicleAckermannGeometryData ackermann;
	ackermann.mAccuracy = 1.0f;
	ackermann.mAxleSeparation = wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).z - wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).z;
	ackermann.mFrontWidth =	wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT).x - wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).x;
	ackermann.mRearWidth = wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_RIGHT).x -	wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).x;
	driveSimData.setAckermannGeometryData(ackermann);

	PxFilterData wheelFilterData;
	wheelFilterData.word0 = EntityFlags::Wheel;

	std::vector<PxVec3> wheelVerts;
	float angle = 0;
	const int circlePoints = 32;
	for (int i = 0; i < circlePoints; ++i)
	{
		float x = cos(angle) * wheelRadius;
		float y = sin(angle) * wheelRadius;
		wheelVerts.emplace_back(wheelWidth / 2, y, x);
		wheelVerts.emplace_back(-wheelWidth / 2, y, x);
		angle += 2 * Float::Pi / circlePoints;
	}

	PxConvexMesh* convexMesh = PxToolkit::createConvexMeshSafe(*physics->backend, *physics->cooking, wheelVerts.data(), 2 * circlePoints, PxConvexFlag::eGPU_COMPATIBLE, 64);
	
	//Add all the wheel shapes to the actor.
	for (PxU32 i = 0; i < numWheels; i++)
	{
		PxConvexMeshGeometry geom{convexMesh};
		PxShape* wheelShape = PxRigidActorExt::createExclusiveShape(*dynamicActor, geom, *physics->defaultMaterial);
		wheelShape->setQueryFilterData(wheelFilterData);
		wheelShape->setSimulationFilterData(wheelFilterData);
		wheelShape->setLocalPose(PxTransform(PxIdentity));
	}
	convexMesh->release();

	PxVehicleDrive4W* wheels = PxVehicleDrive4W::allocate(numWheels);
	wheels->setup(physics->backend, dynamicActor, *wheelsSimData, driveSimData, numWheels - 4);
	wheelsSimData->free();

	rigidActor->userData = this;
	lastPhysicsPosition = transform->WorldPosition();
	lastPhysicsRotation = transform->WorldRotation();
	physics->RegisterRigidBody(this);
}

void Vehicle::PostProcessPhysics()
{
}

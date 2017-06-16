#include "Physics.h"
#include <iostream>
#include <vehicle/PxVehicleSDK.h>
#include "RigidBody.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Mesh.h"
#include "Vehicle.h";
#include "json.h"

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


namespace CollisionOptions
{

	/**
	 * \brief 
	 * Options for contact reporting. Corresponding to PxFilterData.word2.
	 */
	enum Type : PxU32
	{
		NotifyWhenFound = 1u << 0,
		NotifyWhenLost = 1u << 1,
		NotifyWhenPersists = 1u << 2
	};
}

// word0: EntityFlags of the object
// word1: EntityFlags the object wants to collide with
// word2: CollisionOptions
PxFilterFlags FilterShader(
	PxFilterObjectAttributes attributes0,
	PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, 
	PxFilterData filterData1, 
	PxPairFlags& pairFlags,
	const void* constantBlock, 
	PxU32 constantBlockSize)
{
	if (filterData0.word1 & filterData1.word0 || filterData1.word1 & filterData0.word0)
	{
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlag::eDEFAULT;
		}
		
		pairFlags = PxPairFlag::eCONTACT_DEFAULT;

		if (filterData0.word2 & CollisionOptions::NotifyWhenFound || filterData1.word2 & CollisionOptions::NotifyWhenFound)
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		if (filterData0.word2 & CollisionOptions::NotifyWhenLost || filterData1.word2 & CollisionOptions::NotifyWhenLost)
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
		if (filterData0.word2 & CollisionOptions::NotifyWhenPersists || filterData1.word2 & CollisionOptions::NotifyWhenPersists)
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_PERSISTS;

		return PxFilterFlag::eDEFAULT;
	}
	else
		return PxFilterFlag::eSUPPRESS;
}

PxQueryHitType::Enum VehicleRaycastFilterShader(
	PxFilterData queryFilterData,
	PxFilterData objectFilterData,
	const void* constantBlock,
	PxU32 constantBlockSize,
	PxHitFlags& hitFlags)
{
	if (queryFilterData.word1 & objectFilterData.word0)
		return PxQueryHitType::eBLOCK;
	else
		return PxQueryHitType::eNONE;
}

Physics::Physics(int maxVehicles)
	:MaxVehicles{maxVehicles}, MaxWheels{MaxVehicles * PX_MAX_NB_WHEELS}
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

	bool success = PxInitExtensions(*backend, pvd);
	SDL_assert(backend && cooking && success);
	success = PxInitVehicleSDK(*backend);
	SDL_assert(success);

	//defaultMaterial = backend->createMaterial(0.5f, 0.5f, 0.1f);
	auto json = ResourceManager::Instance().LoadJson("Settings/physics.settings");
	SDL_assert(json->IsObject());
	for(auto& mat : (*json)["materials"].GetArray())
	{
		auto material = backend->createMaterial(mat["static_friction"].GetFloat(), mat["dynamic_friction"].GetFloat(), mat["restitution"].GetFloat());
		materials[mat["name"].GetString()] = material;
	}

	PxSceneDesc scnDesc{tolerance};
	cpuDispatcher = PxDefaultCpuDispatcherCreate(4);
	scnDesc.cpuDispatcher = cpuDispatcher;
	scnDesc.filterShader = FilterShader;
	scnDesc.gravity = PxVec3{0, -9.81f, 0};
	scene = backend->createScene(scnDesc);
	scene->getScenePvdClient()->setScenePvdFlags(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS | PxPvdSceneFlag::eTRANSMIT_CONTACTS | PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES);
	scene->setFlag(PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);
	scene->setFlag(PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS, true);

	PxVehicleSetBasisVectors(PxVec3{ 0, 1, 0 }, PxVec3{ 0, 0, 1 });
	PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);

	vehQueryResults = new PxRaycastQueryResult[MaxWheels];
	vehQueryHitBuffer = new PxRaycastHit[MaxWheels];
	PxBatchQueryDesc queryDesc(MaxWheels, 0, 0);
	queryDesc.queryMemory.userRaycastResultBuffer = vehQueryResults;
	queryDesc.queryMemory.userRaycastTouchBuffer = vehQueryHitBuffer;
	queryDesc.queryMemory.raycastTouchBufferSize = MaxWheels;
	queryDesc.preFilterShader = VehicleRaycastFilterShader;
	vehicleQuery = scene->createBatchQuery(queryDesc);

	wantedCollisionsOf[EntityFlags::DefaultPos] = EntityFlags::All;
	wantedCollisionsOf[EntityFlags::PropPos] = EntityFlags::Prop;
	wantedCollisionsOf[EntityFlags::ChassisPos] = EntityFlags::Prop | EntityFlags::Chassis;
	wantedCollisionsOf[EntityFlags::WheelPos] = EntityFlags::Prop;
	wantedCollisionsOf[EntityFlags::GroundPos] = EntityFlags::Prop | EntityFlags::Chassis;



	surfaceToFriction = PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(Tire::Highest, materials.size());
	PxMaterial** surfaceMaterials = new PxMaterial*[materials.size()];
	int matIdx = 0;
	std::unordered_map<std::string, int> matIndices;
	for (auto& material : materials)
	{
		matIndices[material.first] = matIdx;
		surfaceMaterials[matIdx] = material.second;
		++matIdx;
	}

	float* tireMatFriction[Tire::Highest];
	for (int i = 0; i < Tire::Highest; ++i)
	{
		tireMatFriction[i] = new float[materials.size()];
		for (int j = 0; j < materials.size(); ++j)
		{
			tireMatFriction[i][j] = 0.5f;
		}
	}
	for (auto& element : (*json)["tire_friction"].GetArray())
	{
		int tire = Tire::FromString(element["tire_type"].GetString());
		for (auto& mat : element["materials"].GetArray())
		{
			tireMatFriction[tire][matIndices[mat["name"].GetString()]] = mat["friction"].GetFloat();
		}
	}

	PxVehicleDrivableSurfaceType* surfaceTypes = new PxVehicleDrivableSurfaceType[materials.size()];
	for (int i = 0; i < materials.size(); ++i)
	{
		surfaceTypes[i].mType = i;
	}
	surfaceToFriction->setup(Tire::Highest, materials.size(), const_cast<const PxMaterial**>(surfaceMaterials), surfaceTypes);
	for (int i = 0; i < materials.size(); ++i)
	{
		for (int j = 0; j < Tire::Highest; ++j)
		{
			surfaceToFriction->setTypePairFriction(i, j, tireMatFriction[j][i]);
		}
	}

	delete[] surfaceMaterials;
	delete[] surfaceTypes;
	for (int i = 0; i < Tire::Highest; ++i)
	{
		delete[] tireMatFriction[i];
	}

	vehWheelQueryResultBuffer = new PxVehicleWheelQueryResult[MaxVehicles];
	for (int i = 0; i < MaxVehicles; ++i)
	{
		vehWheelQueryResultBuffer[i].wheelQueryResults = new PxWheelQueryResult[PX_MAX_NB_WHEELS];
		vehWheelQueryResultBuffer[i].nbWheelQueryResults = PX_MAX_NB_WHEELS;
	}
}


Physics::~Physics()
{
	for (auto& pair : physicsMeshes)
	{
		pair.second->release();
	}
	for (auto& pair : terrains)
	{
		pair.second->heightField->release();
		delete pair.second;
		pair.second = nullptr;
	}
	for (int i = 0; i < MaxVehicles; ++i)
	{
		delete[] vehWheelQueryResultBuffer[i].wheelQueryResults;
	}
	delete[] vehWheelQueryResultBuffer;
	surfaceToFriction->release();
	vehicleQuery->release();
	delete[] vehQueryHitBuffer;
	delete[] vehQueryResults;
	scene->release();
	for (auto& element : materials)
	{
		element.second->release();
	}
	PxCloseVehicleSDK();
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
	Vehicle* vehicle = dynamic_cast<Vehicle*>(rigidBody);
	if (vehicle)
	{
		wheels.push_back(vehicle->wheels);
		vehicles.push_back(vehicle);
	}
}

void Physics::UnregisterRigidBody(RigidBody* rigidBody)
{
	auto found = std::find(rigidBodies.begin(), rigidBodies.end(), rigidBody);
	if (found != rigidBodies.end())
	{
		scene->removeActor(*rigidBody->rigidActor);
		rigidBodies.erase(found);
		Vehicle* vehicle = dynamic_cast<Vehicle*>(rigidBody);
		if(vehicle)
		{
			auto foundVeh = find(vehicles.begin(), vehicles.end(), vehicle);
			auto dist = std::distance(vehicles.begin(), foundVeh);
			vehicles.erase(foundVeh);
			auto wheelsIt = wheels.begin();
			advance(wheelsIt, dist);
			wheels.erase(wheelsIt);
		}
	}
}

void Physics::Step(float deltaTime)
{
	PxVehicleSuspensionRaycasts(vehicleQuery, wheels.size(), wheels.data(), wheels.size() * PX_MAX_NB_WHEELS, vehQueryResults);
	for (int i = 0; i < vehicles.size(); ++i)
	{
		vehicles[i]->BeforeVehicleUpdate(deltaTime);
	}
	PxVehicleUpdates(deltaTime, scene->getGravity(), *surfaceToFriction, wheels.size(), wheels.data(), vehWheelQueryResultBuffer);
	for (int i = 0; i < vehicles.size(); ++i)
	{
		vehicles[i]->AfterVehicleUpdate(vehWheelQueryResultBuffer[i]);
	}

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
	for (RigidBody* body : rigidBodies)
		body->PostProcessPhysics();
}

void Physics::DebugDraw()
{
}

PxTriangleMesh* Physics::GetMesh(const std::shared_ptr<Mesh>& mesh)
{
	auto found = physicsMeshes.find(mesh);
	if (found == physicsMeshes.end())
	{
		PxCookingParams params{backend->getTolerancesScale()};
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

const Physics::Terrain* Physics::GetTerrain(const std::shared_ptr<Mesh>& mesh)
{
	auto found = terrains.find(mesh);
	if (found == terrains.end())
	{
		int minX = std::numeric_limits<int>::max();
		int minZ = std::numeric_limits<int>::max();
		int maxX = std::numeric_limits<int>::lowest();
		int maxZ = std::numeric_limits<int>::lowest();
		float minY = std::numeric_limits<float>::infinity();
		float maxY = std::numeric_limits<float>::lowest();
		for (int i = 0; i < mesh->vertices.size(); ++i)
		{
			int x = std::round(mesh->vertices[i].x);
			int z = std::round(mesh->vertices[i].z);
			float y = mesh->vertices[i].y;
			if (x < minX)
				minX = x;
			if (x > maxX)
				maxX = x;
			if (y < minY)
				minY = y;
			if (y > maxY)
				maxY = y;
			if (z < minZ)
				minZ = z;
			if (z > maxZ)
				maxZ = z;
		}

		int rows = maxX - minX + 1;
		int columns = maxZ - minZ + 1;
		float height = maxY - minY;
		SDL_assert(rows * columns && rows * columns == mesh->vertices.size());

		int8_t* valueAssigned = new int8_t[rows * columns]{};
		PxHeightFieldSample* heightFieldSamples = new PxHeightFieldSample[rows * columns];
		for (int i = 0; i < mesh->vertices.size(); ++i)
		{
			int row = -minX + std::round(mesh->vertices[i].x);
			int column = -minZ + std::round(mesh->vertices[i].z);
			int index = row * columns + column;
			SDL_assert(!valueAssigned[index]);
			int16_t quantizedHeight = static_cast<int16_t>((mesh->vertices[i].y - minY) / height * ((1 << 16) - 1) + std::numeric_limits<int16_t>::lowest());
			auto& data = heightFieldSamples[index];
			data.height = quantizedHeight;
			data.materialIndex0 = 0;
			data.materialIndex1 = 0;
			data.clearTessFlag();
			valueAssigned[index] = 1;
		}

		PxHeightFieldDesc desc{};
		desc.format = PxHeightFieldFormat::eS16_TM;
		desc.nbRows = rows;
		desc.nbColumns = columns;
		desc.samples.data = heightFieldSamples;
		desc.samples.stride = sizeof(PxHeightFieldSample);

		PxHeightField* heightField = cooking->createHeightField(desc, backend->getPhysicsInsertionCallback());

		delete[] heightFieldSamples;
		delete[] valueAssigned;

		Terrain* terrain = new Terrain;
		terrain->height = height;
		terrain->heightField = heightField;
		terrain->minX = minX;
		terrain->minZ = minZ;
		terrain->minHeight = minY;

		terrains.insert(found, make_pair(mesh, terrain));
		return terrain;
	}

	return found->second;
}

void Physics::SetUpFilterData(physx::PxFilterData& filterData, EntityFlags::Type entityType)
{
	filterData.word0 = entityType;
	for (int i = 0; i <= EntityFlags::HighestPos; ++i)
	{
		if (entityType & (1u << i))
			filterData.word1 |= wantedCollisionsOf[i];
	}
}

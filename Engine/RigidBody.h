#pragma once
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <PxPhysicsAPI.h>
#include "Component.h"
#include "core.h"
#include "Mesh.h"
#include "math_utility.h"

class Entity;
class Transform;

class RigidBody : public Component
{
	DELETE_COPY_MOVE(RigidBody)
	friend class Physics;
public:
	enum class Shape
	{
		Box, Sphere, Mesh, Convex, Terrain
	};

	explicit RigidBody(Entity& entity);
	virtual ~RigidBody();
	
	int Deserialize(const Json& json) override;
	void Initialize() override;
	void OnMessageReceived(Entity* origin, Message* message) override;
	void SetShape(Shape shape);
	Shape GetShape() const { return shapeType; }
	void SetDensity(float mass);
	float Density() const { return density; }
	glm::vec3 Velocity() const;
	void AddForce(const glm::vec3 force, physx::PxForceMode::Enum mode = physx::PxForceMode::eFORCE);
	void AddTorque(const glm::vec3 torque, physx::PxForceMode::Enum mode = physx::PxForceMode::eFORCE);
	float Mass() const { return rigidDynamic ? rigidDynamic->getMass() : 0; }
	glm::vec3 MassSpaceInertiaTensor() const { return rigidDynamic? ToVec3(rigidDynamic->getMassSpaceInertiaTensor()) : glm::vec3{}; }

protected:
	Transform* transform{};
	Shape shapeType{};
	glm::vec3 halfSize{0.5f};
	std::shared_ptr<Mesh> mesh;
	float density{ 500 };
	glm::quat lastPhysicsRotation;
	glm::vec3 lastPhysicsPosition;
	static Physics* physics;
	physx::PxShape* shape{};
	physx::PxRigidActor* rigidActor{};
	physx::PxRigidDynamic* rigidDynamic{};
	bool isStatic{};
	physx::PxFilterData filterData;
	glm::vec3 offset;
	physx::PxMaterial* material{};

	virtual void Update();
	static std::unordered_map<std::string, Shape> stringToShape;
	virtual void PostProcessPhysics();
	void SetShapeInternal(Shape shape);
	virtual void UpdateMassAndInertia();
	virtual void UpdateTransform();
	void InitCommonProps();
};

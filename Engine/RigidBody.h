#pragma once
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <PxPhysicsAPI.h>
#include "Component.h"
#include "core.h"
#include "Mesh.h"

class Entity;
class Transform;

class RigidBody : public Component
{
	DELETE_COPY_MOVE(RigidBody)
	friend class Physics;
public:
	enum class Shape
	{
		Box, Sphere, Mesh, Terrain
	};

	explicit RigidBody(Entity* entity);
	virtual ~RigidBody();
	
	void Deserialize(const Json& json) override;
	void Initialize() override;
	void OnMessageReceived(Entity* origin, Message* message) override;
	void SetShape(Shape shape);
	Shape GetShape() const { return shapeType; }
	void SetDensity(float mass);
	float Density() const { return density; }	

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
	bool isStatic{};
	physx::PxFilterData filterData;
	glm::vec3 offset;

	virtual void Update();
	static std::unordered_map<std::string, Shape> stringToShape;
	virtual void PostProcessPhysics();
	void SetShapeInternal(Shape shape);
	void SetDensityInternal(float density);
	virtual void UpdateTransform();
	void InitCommonProps();
};

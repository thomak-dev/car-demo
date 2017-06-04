#pragma once
#include <unordered_map>
#include <memory>
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Component.h"
#include "core.h"
#include "Mesh.h"

class Entity;
class Transform;

class RigidBody : public Component, public btMotionState
{
	DELETE_COPY_MOVE(RigidBody)
	friend class Physics;
public:
	enum class Shape
	{
		Box, Sphere, Mesh
	};

	explicit RigidBody(Entity* entity);
	virtual ~RigidBody();
	
	void Deserialize(const Json& json) override;
	void Initialize() override;
	void OnMessageReceived(Entity* origin, Message* message) override;
	void getWorldTransform(btTransform& worldTrans) const override;
	void setWorldTransform(const btTransform& worldTrans) override;
	void SetShape(Shape shape);
	void SetMass(float mass);

private:
	Transform* transform{};
	btRigidBody* bulletRigidBody{};
	btCollisionShape* shape{};
	Shape shapeType{};
	glm::vec3 halfSize{0.5f};
	std::shared_ptr<Mesh> mesh;
	float mass{ 1 };
	glm::quat lastPhysicsRotation;
	glm::vec3 lastPhysicsPosition;

	void Update();
	btCollisionShape* GenerateShape(Shape type) const;
	static std::unordered_map<std::string, Shape> stringToShape;
};

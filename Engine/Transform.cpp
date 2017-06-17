#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Entity.h"
#include "math_utility.h"

glm::mat4 Transform::ParentWorldMatrix() const
{
	auto parent = entity.Parent();
	if (parent)
	{
		Transform* parentTrans = parent->GetComponentInAncestors<Transform>();
		if (parentTrans)
			return parentTrans->WorldMatrix();
	}
	return glm::mat4{};
}

glm::mat4 Transform::ParentInverseWorldMatrix() const
{
	auto parent = entity.Parent();
	if (parent)
	{
		Transform* parentTrans = parent->GetComponentInAncestors<Transform>();
		if (parentTrans)
			return parentTrans->InverseWorldMatrix();
	}
	return glm::mat4{};
}

glm::mat4 Transform::WorldMatrix() const
{
	return ParentWorldMatrix() * Matrix();
}

glm::mat4 Transform::InverseWorldMatrix() const
{
	return InverseMatrix() * ParentInverseWorldMatrix();
}

glm::mat4 Transform::Matrix() const
{
	glm::mat4 trans
	{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		position.x, position.y, position.z, 1
	};
	return glm::scale(trans * glm::mat4_cast(rotation), scale);
}

glm::mat4 Transform::InverseMatrix() const
{
	glm::mat4 scaleMat
	{
		1/scale.x,0,0,0,
		0,1/scale.y,0,0,
		0,0,1/scale.z,0,
		0,0,0,1
	};
	return glm::translate(scaleMat * glm::mat4_cast(inverse(rotation)), -position);
}

void Transform::SetMatrix(const glm::mat4x4 matrix)
{
	glm::quat rot{ glm::uninitialize };
	DecomposeMatrix(matrix, position, scale, rot);
	SetRotation(rot);
}

glm::vec3 Transform::WorldPosition() const
{
	return ParentWorldMatrix() * glm::vec4(position, 1);
}

void Transform::SetWorldPosition(float x, float y, float z)
{
	SetWorldPosition(glm::vec3(x, y, z));
}

void Transform::SetWorldPosition(const glm::vec3& position)
{
	SetPosition(ParentInverseWorldMatrix() * glm::vec4(position, 1));
}

void Transform::SetPosition(float x, float y, float z)
{
	SetPosition(glm::vec3(x, y, z));
}

void Transform::SetPosition(const glm::vec3& position)
{

	this->position = position;
}

glm::vec3 Transform::WorldScale() const
{
	glm::mat4 parent2World = ParentWorldMatrix();
	return glm::vec3(length(parent2World[0]), length(parent2World[1]), length(parent2World[2])) * scale;
}

void Transform::SetWorldScale(float x, float y, float z)
{
	SetWorldScale(glm::vec3(x, y, z));
}

void Transform::SetWorldScale(const glm::vec3& scale)
{
	glm::mat4 world2parent = ParentInverseWorldMatrix();
	SetScale(glm::vec3(length(world2parent[0]), length(world2parent[1]), length(world2parent[2])) * scale);
}

void Transform::SetScale(float x, float y, float z)
{
	SetScale(glm::vec3(x, y, z));
}

void Transform::SetScale(const glm::vec3& scale)
{
	this->scale = scale;
}

glm::quat Transform::WorldRotation() const
{
	glm::quat parentWorldRot{ glm::uninitialize };
	glm::vec3 pos{ glm::uninitialize };
	glm::vec3 scale{ glm::uninitialize };
	DecomposeMatrix(ParentWorldMatrix(), pos, scale, parentWorldRot);
	return parentWorldRot * rotation;
}

void Transform::SetWorldRotation(float pitch, float yaw, float roll)
{
	SetWorldRotation(glm::quat(glm::vec3(pitch, yaw, roll)));
}

void Transform::SetWorldRotation(const glm::quat& rotation)
{
	glm::quat parentInverseWorldRot{ glm::uninitialize };
	glm::vec3 pos{ glm::uninitialize };
	glm::vec3 scale{ glm::uninitialize };
	DecomposeMatrix(ParentInverseWorldMatrix(), pos, scale, parentInverseWorldRot);
	SetRotation(parentInverseWorldRot * rotation);
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	SetRotation(glm::quat(glm::vec3(pitch, yaw, roll)));
}

void Transform::SetRotation(const glm::quat& rotation)
{
	this->rotation = normalize(rotation);
}

glm::vec3 Transform::Forward() const
{
	return normalize(rotation * Vector3::Forward);
}

glm::vec3 Transform::Left() const
{
	return normalize(rotation * Vector3::Left);
}

glm::vec3 Transform::Up() const
{
	return normalize(rotation * Vector3::Up);
}

void Transform::RotateLocally(float angle, const glm::vec3& axis)
{
	SetRotation(rotation * glm::angleAxis(angle, axis));
}

void Transform::RotateGlobally(float angle, const glm::vec3& axis)
{
	SetRotation(rotation * glm::angleAxis(angle, inverse(rotation) * axis));
}

void Transform::TranslateGlobally(const glm::vec3& translation)
{
	position += translation;
}

void Transform::TranslateLocally(const glm::vec3& translation)
{
	position += rotation * translation;
}

int Transform::Deserialize(const Json& json)
{
	int count = 0;
	if (json.HasMember("position") && ++count)
		SetPosition(ToVec3(json["position"]));
	if (json.HasMember("rotation") && ++count)
		SetRotation(radians(ToVec3(json["rotation"])));
	if (json.HasMember("scale") && ++count)
		SetScale(ToVec3(json["scale"]));
	return count;
}
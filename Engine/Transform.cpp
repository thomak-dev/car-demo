#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Entity.h"
#include "math_utility.h"

glm::mat4 Transform::WorldMatrix() const
{
	if (entity->Parent() == nullptr)
		return Matrix();
	else
	{
		Transform* trans = entity->Parent()->GetComponentInAncestors<Transform>();
		if (trans)
		{
			return trans->WorldMatrix() * Matrix();
		}
		else
			return Matrix();
	}
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

void Transform::SetMatrix(const glm::mat4x4 matrix)
{
	position = matrix[3];
	SetRotation(glm::quat_cast(matrix));
	scale = glm::vec3(glm::length(matrix[0]), glm::length(matrix[1]), glm::length(matrix[2]));
}

glm::vec3 Transform::WorldPosition() const
{
	return WorldMatrix()[3];
}

void Transform::SetWorldPosition(float x, float y, float z)
{
	SetWorldPosition(glm::vec3(x, y, z));
}

void Transform::SetWorldPosition(const glm::vec3& position)
{
	Transform* parentTransform;
	if (entity->Parent() && ((parentTransform = entity->Parent()->GetComponentInAncestors<Transform>())))
		SetPosition(affineInverse(parentTransform->WorldMatrix()) * glm::vec4(position, 1));
	else
		SetPosition(position);
}

void Transform::SetPosition(float x, float y, float z)
{
	position = glm::vec3(x, y, z);
}

void Transform::SetPosition(const glm::vec3& position)
{
	this->position = position;
}

glm::vec3 Transform::WorldScale() const
{
	const glm::mat3& worldMatrix{ WorldMatrix() };
	return glm::vec3(length(worldMatrix[0]), length(worldMatrix[1]), length(worldMatrix[2]));
}

void Transform::SetWorldScale(float x, float y, float z)
{
	SetWorldScale(glm::vec3(x, y, z));
}

void Transform::SetWorldScale(const glm::vec3& scale)
{
	Transform* parentTransform;
	if (entity->Parent() && ((parentTransform = entity->Parent()->GetComponentInAncestors<Transform>())))
	{
		const glm::mat3& inverse = affineInverse(parentTransform->WorldMatrix());
		glm::vec3 invScale{length(inverse[0]), length(inverse[1]), length(inverse[2])};
		SetScale(invScale * scale);
	}
	else
		SetScale(scale);
}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
}

void Transform::SetScale(const glm::vec3& scale)
{
	this->scale = scale;
}

glm::quat Transform::WorldRotation() const
{
	return glm::quat_cast(WorldMatrix());
}

void Transform::SetWorldRotation(float pitch, float yaw, float roll)
{
	SetWorldRotation(glm::quat(glm::vec3(pitch, yaw, roll)));
}

void Transform::SetWorldRotation(const glm::quat& rotation)
{
	Transform* parentTransform;
	if (entity->Parent() && ((parentTransform = entity->Parent()->GetComponentInAncestors<Transform>())))
		SetRotation(quat_cast(affineInverse(parentTransform->WorldMatrix())) * rotation);
	else
		SetRotation(rotation);
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation = glm::quat(glm::vec3(pitch, yaw, roll));
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

void Transform::Deserialize(const Json& json)
{
	if (json.HasMember("position"))
		SetPosition(Vec3FromJson(json["position"]));
	if (json.HasMember("rotation"))
		SetRotation(radians(Vec3FromJson(json["rotation"])));
	if (json.HasMember("scale"))
		SetScale(Vec3FromJson(json["scale"]));
}

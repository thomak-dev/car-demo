#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Updatable.h"

class Transform : public Updatable
{
	DELETE_COPY_MOVE(Transform)
public:
	using Updatable::Updatable;
	Transform() = delete;
	virtual ~Transform() = default;


	void Initialize() override;
	glm::mat4 ParentWorldMatrix() const;
	glm::mat4 ParentInverseWorldMatrix() const;
	glm::mat4 WorldMatrix() const;
	glm::mat4 InverseWorldMatrix() const;
	glm::mat4 Matrix() const;
	glm::mat4 InverseMatrix() const;
	void SetMatrix(const glm::mat4x4 matrix);
	glm::vec3 WorldPosition() const;
	glm::vec3 Position() const { return position; }
	void SetWorldPosition(float, float, float);
	void SetWorldPosition(const glm::vec3& position);
	void SetPosition(float, float, float);
	void SetPosition(const glm::vec3& position);
	glm::vec3 WorldScale() const;
	glm::vec3 Scale() const { return scale; }
	void SetWorldScale(float, float, float);
	void SetWorldScale(const glm::vec3& scale);
	void SetScale(float, float, float);
	void SetScale(const glm::vec3& scale);
	glm::quat WorldRotation() const;
	glm::quat Rotation() const { return rotation; }
	void SetWorldRotation(float, float, float);
	void SetWorldRotation(const glm::quat& rotation);
	void SetRotation(float, float, float);
	void SetRotation(const glm::quat& rotation);
	glm::vec3 Forward() const;
	glm::vec3 Left() const;
	glm::vec3 Up() const;
	void RotateLocally(float angle, const glm::vec3& axis);
	void RotateGlobally(float angle, const glm::vec3& axis);
	void TranslateGlobally(const glm::vec3& translation);
	void TranslateLocally(const glm::vec3& translation);
	int Deserialize(const Json& json) override;
	glm::vec3 GetVelocity() const { return velocity; }

protected:
	void Update(float deltaTime) override;

private:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale {1, 1, 1};
	glm::vec3 lastPos;
	glm::vec3 velocity;
};


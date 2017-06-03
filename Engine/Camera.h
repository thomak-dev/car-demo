#pragma once
#include <unordered_map>
#include <glm/fwd.hpp>
#include <glm/vec4.hpp>
#include "core.h"
#include "Component.h"

class Transform;

class Camera : public Component
{
	DELETE_COPY_MOVE(Camera)
public:
	enum class Projection
	{
		PerspectiveAspect,
		OrthographicAspect,
		OrthographicScreen,
		OrthographicNormalized
	};

	Camera() = delete;
	explicit Camera(Entity*);
	virtual ~Camera();

	glm::mat4 ViewMatrix() const;
	glm::mat4 ProjMatrix() const;
	void Initialize() override;

	Projection projection { Projection::PerspectiveAspect };
	float verticalParam{ 60 };
	float farPlane{ 1000 };
	float nearPlane { 0.01f };
	bool clearDepth{ true };
	bool clearColor{ true };
	glm::vec4 background{ 1, 0, 1, 1 };
	void Deserialize(const Json& json) override;
	bool IsMain() const	{ return isMain; }
	void SetIsMain(bool isMain) { this->isMain = isMain; }

private:
	bool isMain{false};
	Transform* transform{};
	static std::unordered_map<std::string, Projection> stringToProjection;
};


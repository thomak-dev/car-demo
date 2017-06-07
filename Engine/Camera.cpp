#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Renderer.h"
#include "Transform.h"
#include "math_utility.h"
#include "Entity.h"
#include "GameWindow.h"

std::unordered_map<std::string, Camera::Projection> Camera::stringToProjection
{
	{"PerspectiveAspect", Camera::Projection::PerspectiveAspect},
	{"OrthographicAspect", Camera::Projection::OrthographicAspect},
	{"OrthographicScreen", Camera::Projection::OrthographicScreen},
	{"OrthographicNormalized", Camera::Projection::OrthographicNormalized}
};

Camera::Camera(Entity* entity)
	: Component{entity}
{
	Renderer::Instance().RegisterCamera(this);
}

Camera::~Camera()
{
	Renderer::Instance().UnregisterCamera(this);
}

glm::mat4 Camera::ViewMatrix() const
{
	return affineInverse(rotate(transform->WorldMatrix(), Float::Pi, Vector3::Up));
}

glm::mat4 Camera::ProjMatrix() const
{
	struct
	{
		float w;
		float h;
	} screen{GameWindow::Instance().Width(), GameWindow::Instance().Height()};

	if (projection == Projection::OrthographicNormalized)
		return glm::ortho<float>(-1, 1, -1, 1, nearPlane, farPlane);
	if (projection == Projection::OrthographicScreen)
		return glm::ortho<float>(0, -screen.w, 0, screen.h, nearPlane, farPlane);
	if (projection == Projection::OrthographicAspect)
	{
		float horizontalExtent = verticalParam * screen.w / screen.h / 2;
		float verticalExtent = verticalParam / 2;
		return glm::ortho<float>(-horizontalExtent, horizontalExtent, -verticalExtent, verticalExtent, nearPlane, farPlane);
	}
	//if (projection == Projection::PerspectiveAspect)
	return glm::perspective(glm::radians(verticalParam), screen.w / screen.h, nearPlane, farPlane);
}

void Camera::Initialize()
{
	Component::Initialize();
	transform = entity->GetComponent<Transform>();
}

void Camera::Deserialize(const Json& json)
{
	if (json.HasMember("vertical_param"))
		verticalParam = json["vertical_param"].GetFloat();
	if (json.HasMember("projection"))
		projection = stringToProjection[json["projection"].GetString()];
	if (json.HasMember("near"))
		nearPlane = json["near"].GetFloat();
	if (json.HasMember("far"))
		farPlane = json["far"].GetFloat();
	if (json.HasMember("clear_depth"))
		clearDepth = json["clear_depth"].GetBool();
	if (json.HasMember("clear_color"))
		clearColor = json["clear_color"].GetBool();
	if (json.HasMember("background"))
		background = Vec4FromJson(json["background"]);
	if (json.HasMember("main"))
		isMain = json["main"].GetBool();
}

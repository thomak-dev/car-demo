#include "json.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

glm::vec3 Vec3FromJson(const rapidjson::Value& json)
{
	const auto& arr = json.GetArray();
	return glm::vec3(arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat());
}

glm::vec4 Vec4FromJson(const rapidjson::Value& json)
{
	const auto& arr = json.GetArray();
	return glm::vec4(arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat(), arr[3].GetFloat());
}
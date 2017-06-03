#pragma once
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <glm/fwd.hpp>

glm::vec3 Vec3FromJson(const rapidjson::Value& json);
glm::vec4 Vec4FromJson(const rapidjson::Value& json);
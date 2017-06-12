#pragma once
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <glm/fwd.hpp>

glm::vec3 ToVec3(const rapidjson::Value& json);
glm::vec4 ToVec4(const rapidjson::Value& json);
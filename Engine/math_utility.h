#pragma once
#include <glm/fwd.hpp>
#include <assimp/types.h>
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

namespace Vector3
{
	extern const glm::vec3 Up;
	extern const glm::vec3 Forward;
	extern const glm::vec3 Left;
}

namespace Float
{
	constexpr float Pi = static_cast<float>(3.141592653589793238462643383279502884197169399375105820974);
}

// ceil to multiple of a power of 2, useful for alignment (po2 example values: 1,2,4,8,16)
unsigned CeilToBoundary(unsigned x, unsigned po2);

// floor to multiple of a power of 2, useful for alignment (po2 example values: 1,2,4,8,16)
unsigned FloorToBoundary(unsigned x, unsigned po2);

void AiToGlmMatrix(glm::mat4& result, const aiMatrix4x4& aiMat);

void BtToGlmVec3(glm::vec3& result, const btVector3& btVec);
void GlmToBtVec3(btVector3& result, const glm::vec3& glmVec);
glm::vec3 BtToGlmVec3(const btVector3& btVec);
btVector3 GlmToBtVec3(const glm::vec3& glmVec);
void BtToGlmQuat(glm::quat& result, const btQuaternion& btQuat);

int CeilLog2(uint64_t x);

// x must be less or equal to 31
unsigned Pow2(unsigned x);
void DecomposeMatrix(const glm::mat4& matrix, glm::vec3& position, glm::vec3& scale, glm::quat& rotation);
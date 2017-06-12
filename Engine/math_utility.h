#pragma once
#include <glm/fwd.hpp>
#include <assimp/types.h>
#include <PxPhysicsAPI.h>

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

physx::PxVec3 ToPxVec3(const glm::vec3& vec);
physx::PxVec4 ToPxVec4(const glm::vec4& vec);
physx::PxQuat ToPxQuat(const glm::quat& quat);
physx::PxMat44 ToPxMat44(const glm::mat4& mat);
glm::vec3 ToVec3(const physx::PxVec3& vec);
glm::quat ToQuat(const physx::PxQuat& quat);

int CeilLog2(uint64_t x);

// x must be less or equal to 31
unsigned Pow2(unsigned x);
void DecomposeMatrix(const glm::mat4& matrix, glm::vec3& position, glm::vec3& scale, glm::quat& rotation);
#include "math_utility.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>


const glm::vec3 Vector3::Up{ 0, 1, 0 };
const glm::vec3 Vector3::Forward{ 0, 0, 1 };
const glm::vec3 Vector3::Left{ 1, 0, 0 };

unsigned CeilToBoundary(unsigned x, unsigned po2)
{
#pragma warning (suppress : 4146) // suppress unsigned unary minus warning
	return (x + po2 - 1) & -po2;
}

unsigned FloorToBoundary(unsigned x, unsigned po2)
{
#pragma warning (suppress : 4146) // suppress unsigned unary minus warning
	return x & -po2;
}

void AiToGlmMatrix(glm::mat4& result, const aiMatrix4x4& aiMat)
{
	result[0] = glm::vec4(aiMat[0][0], aiMat[1][0], aiMat[2][0], aiMat[3][0]);
	result[1] = glm::vec4(aiMat[0][1], aiMat[1][1], aiMat[2][1], aiMat[3][1]);
	result[2] = glm::vec4(aiMat[0][2], aiMat[1][2], aiMat[2][2], aiMat[3][2]);
	result[3] = glm::vec4(aiMat[0][3], aiMat[1][3], aiMat[2][3], aiMat[3][3]);
}

physx::PxVec3 ToPxVec3(const glm::vec3& vec)
{
	return physx::PxVec3{ vec.x, vec.y, vec.z };
}

physx::PxVec4 ToPxVec4(const glm::vec4& vec)
{
	return physx::PxVec4{ vec.x, vec.y, vec.z, vec.w };
}

physx::PxQuat ToPxQuat(const glm::quat& quat)
{
	return physx::PxQuat(quat.x, quat.y, quat.z, quat.w);
}

physx::PxMat44 ToPxMat44(const glm::mat4& mat)
{
	physx::PxMat44 pMat;
	pMat[0] = ToPxVec4(mat[0]);
	pMat[1] = ToPxVec4(mat[1]);
	pMat[2] = ToPxVec4(mat[2]);
	pMat[3] = ToPxVec4(mat[3]);
	return pMat;
}

glm::vec3 ToVec3(const physx::PxVec3& vec)
{
	return glm::vec3{ vec.x, vec.y, vec.z };
}

glm::quat ToQuat(const physx::PxQuat& quat)
{
	return glm::quat{ quat.w, quat.x, quat.y, quat.z };
}

int CeilLog2(uint64_t x)
{
	static const uint64_t t[6] = {
		0xFFFFFFFF00000000ull,
		0x00000000FFFF0000ull,
		0x000000000000FF00ull,
		0x00000000000000F0ull,
		0x000000000000000Cull,
		0x0000000000000002ull
	};

	int y = (((x & (x - 1)) == 0) ? 0 : 1);
	int j = 32;

	for (int i = 0; i < 6; i++) {
		int k = (((x & t[i]) == 0) ? 0 : j);
		y += k;
		x >>= k;
		j >>= 1;
	}

	return y;
}

unsigned Pow2(unsigned x)
{
	PRO_ASSERT(x <= 31);
	return 1u << x;
}

// works for affine matrices without odd negative scale
void DecomposeMatrix(const glm::mat4& matrix, glm::vec3& position, glm::vec3& scale, glm::quat& rotation)
{
	position = matrix[3];
	scale = glm::vec3(glm::length(matrix[0]), glm::length(matrix[1]), glm::length(matrix[2]));
	//if (glm::determinant(matrix) < 0)
	//	if (scale.x != 0)
	//		scale.x *= -1;
	//	else if (scale.y != 0)
	//		scale.y *= -1;
	//	else
	//		scale.z *= -1;
	glm::mat3 rot{ matrix };
	rot[0] /= scale.x;
	rot[1] /= scale.y;
	rot[2] /= scale.z;

	rotation = glm::quat_cast(rot);
}
#include "math_utility.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <SDL.h>

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

void BtToGlmVec3(glm::vec3& result, const btVector3& btVec)
{
	result.x = btVec.x();
	result.y = btVec.y();
	result.z = btVec.z();
}

void GlmToBtVec3(btVector3& result, const glm::vec3& glmVec)
{
	result.setX(glmVec.x);
	result.setY(glmVec.y);
	result.setZ(glmVec.z);
}

glm::vec3 BtToGlmVec3(const btVector3& btVec)
{
	glm::vec3 result{ glm::uninitialize };
	BtToGlmVec3(result, btVec);
	return result;
}

btVector3 GlmToBtVec3(const glm::vec3& glmVec)
{
	btVector3 result;
	GlmToBtVec3(result, glmVec);
	return result;
}

void BtToGlmQuat(glm::quat& result, const btQuaternion& btQuat)
{
	result.w = btQuat.w();
	result.x = btQuat.x();
	result.y = btQuat.y();
	result.z = btQuat.z();
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
	SDL_assert(x <= 31);
	return 1u << x;
}
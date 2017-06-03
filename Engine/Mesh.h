#pragma once
#include <vector>
#include <GL/glew.h>
#include <glm/fwd.hpp>
#include <btBulletDynamicsCommon.h>

class Mesh : public btStridingMeshInterface
{
public:
	Mesh() = default;
	Mesh(const Mesh &other);
	Mesh(Mesh &&other) noexcept;
	~Mesh();

	Mesh& operator=(const Mesh& mesh);
	Mesh& operator=(Mesh&& mesh) noexcept;

	std::vector<glm::vec3> vertices{};
	std::vector<glm::vec3> normals{};
	std::vector<glm::vec3> tangents{};
	std::vector<size_t> indices{};
	std::vector<glm::vec2> uvs{};

	void Bind();
	void MarkDirty();

	void getLockedVertexIndexBase(unsigned char** vertexbase, int& numverts, PHY_ScalarType& type, int& stride, unsigned char** indexbase, int& indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart) override;
	void getLockedReadOnlyVertexIndexBase(const unsigned char** vertexbase, int& numverts, PHY_ScalarType& type, int& stride, const unsigned char** indexbase, int& indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart) const override;
	void unLockVertexBase(int subpart) override;
	void unLockReadOnlyVertexBase(int subpart) const override;
	int getNumSubParts() const override;
	void preallocateVertices(int numverts) override;
	void preallocateIndices(int numindices) override;

	friend void swap(Mesh& first, Mesh& second) noexcept;
private:
	GLuint vbo{};
	GLuint ebo{};
	bool dirty{true};

	void GetVertexParameters(int& numverts, PHY_ScalarType& type, int& stride, int& indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart) const;
	void Upload();
	void CreateBuffersIfNeeded();
};


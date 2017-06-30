#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <SDL.h>
#include "core.h"

Mesh::Mesh(const Mesh& other)
	:vertices{other.vertices},
	normals{other.normals},
	indices{other.indices}
{
}

Mesh::Mesh(Mesh&& other) noexcept
	:Mesh{}
{
	swap(*this, other);
}

Mesh::~Mesh()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

Mesh& Mesh::operator=(const Mesh& mesh)
{
	Mesh tmp{ mesh };
	swap(*this, tmp);
	return *this;
}

Mesh& Mesh::operator=(Mesh&& mesh) noexcept
{
	swap(*this, mesh);
	return *this;
}

void Mesh::Bind()
{
	CreateBuffersIfNeeded();
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	if (dirty)
		Upload();
}

void Mesh::MarkDirty()
{
	dirty = true;
}

void Mesh::Upload()
{
	PRO_ASSERT(vertices.size() == normals.size());
	PRO_ASSERT(indices.size() % 3 == 0);

	size_t sizeVertices = sizeof(decltype(vertices)::value_type) * vertices.size();
	size_t sizeTangents = sizeof(decltype(tangents)::value_type) * tangents.size();
	size_t sizeUvs = sizeof(decltype(uvs)::value_type) * uvs.size();

	glBufferData(GL_ARRAY_BUFFER, sizeVertices * 2 + sizeUvs + sizeTangents, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVertices, vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, sizeVertices, sizeVertices, normals.data());
	glBufferSubData(GL_ARRAY_BUFFER, sizeVertices * 2, sizeTangents, tangents.data());
	glBufferSubData(GL_ARRAY_BUFFER, sizeVertices * 2 + sizeTangents, sizeUvs, uvs.data());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(decltype(indices)::value_type) * indices.size(), indices.data(), GL_STATIC_DRAW);

	dirty = false;
}

void Mesh::CreateBuffersIfNeeded()
{
	if (!vbo)
	{
		PRO_ASSERT(!ebo);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);
	}
}


void swap(Mesh& first, Mesh& second) noexcept
{
	using std::swap;
	swap(first.vertices, second.vertices);
	swap(first.normals, second.normals);
	swap(first.tangents, second.tangents);
	swap(first.indices, second.indices);
	swap(first.uvs, second.uvs);
	swap(first.vbo, second.vbo);
	swap(first.ebo, second.ebo);
	swap(first.dirty, second.dirty);
}

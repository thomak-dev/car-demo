#pragma once
#include <vector>
#include <GL/glew.h>
#include <glm/fwd.hpp>

class Mesh
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

	friend void swap(Mesh& first, Mesh& second) noexcept;
private:
	GLuint vbo{};
	GLuint ebo{};
	bool dirty{true};

	void Upload();
	void CreateBuffersIfNeeded();
};


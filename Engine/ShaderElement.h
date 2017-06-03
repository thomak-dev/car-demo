#pragma once
#include <string>
#include <GL/glew.h>
#include "core.h"

class ShaderElement
{
	DELETE_COPY(ShaderElement)
public:
	enum class Type : GLenum
	{
		Vertex = GL_VERTEX_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Geometry = GL_GEOMETRY_SHADER
	};

	ShaderElement(const std::string& path, Type type);
	ShaderElement(ShaderElement&& other);
	~ShaderElement();
	ShaderElement& operator=(ShaderElement&& other);

	GLuint GetShaderObject() const { return shader; };
	bool IsValid();

	friend void swap(ShaderElement& first, ShaderElement& second);
	friend std::ostream& operator<<(std::ostream&, Type type);
private:
	ShaderElement() = default;
	GLuint shader;
};


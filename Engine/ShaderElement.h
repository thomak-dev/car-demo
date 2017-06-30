#pragma once
#include <string>
#include "static_glew.h"
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
	ShaderElement(ShaderElement&& other) noexcept;
	~ShaderElement();
	ShaderElement& operator=(ShaderElement&& other) noexcept;

	GLuint GetShaderObject() const { return shader; };
	bool IsValid() const;

	friend void swap(ShaderElement& first, ShaderElement& second) noexcept;
	friend std::ostream& operator<<(std::ostream&, Type type);
private:
	ShaderElement() = default;
	GLuint shader;
};


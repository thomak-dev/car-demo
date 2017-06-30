#include "ShaderElement.h"
#include <iostream>
#include "core.h"
#include "ResourceManager.h"

bool DidShaderCompile(GLuint shader, std::string *compileLog)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	bool result = status == GL_TRUE;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &status);
	if (status && compileLog)
	{
		GLchar *logBuffer = new GLchar[status];
		glGetShaderInfoLog(shader, status, nullptr, logBuffer);
		*compileLog = logBuffer;
		delete[] logBuffer;
	}
	return result;
}

ShaderElement::ShaderElement(const std::string& source, Type type)
{
	shader = glCreateShader(static_cast<GLenum>(type));
	std::cout << "Compiling " << type << " shader...\n";
	const char* cSource = source.c_str();
	glShaderSource(shader, 1, &cSource, nullptr);
	glCompileShader(shader);
	std::string log{};
	if (!DidShaderCompile(shader, &log))
		std::cout << "Shader couldn't be compiled!\n";
	std::cout << "Log:\n" << log << std::endl;
}

ShaderElement::ShaderElement(ShaderElement&& other) noexcept
	:ShaderElement{}
{
	swap(*this, other);
}

ShaderElement::~ShaderElement()
{
	glDeleteShader(shader);
}

ShaderElement& ShaderElement::operator=(ShaderElement&& other) noexcept
{
	swap(*this, other);
	return *this;
}

bool ShaderElement::IsValid() const
{
	return shader != 0;
}

void swap(ShaderElement& first, ShaderElement& second) noexcept
{
	using std::swap;
	swap(first.shader, second.shader);
}

std::ostream& operator<<(std::ostream& os, ShaderElement::Type type)
{
	switch (type)
	{
	case ShaderElement::Type::Vertex:
		os << "Vertex";
		break;
	case ShaderElement::Type::Fragment:
		os << "Fragment";
		break;
	case ShaderElement::Type::Geometry:
		os << "Geometry";
		break;
	default:
		os << "Unknown";
		break;
	}
	return os;
}

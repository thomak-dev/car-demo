#include "ShaderProgram.h"
#include <iostream>
#include "Shader.h"
#include "ShaderElement.h"
#include "Renderer.h"

bool DidProgramLink(GLuint program, std::string *linkLog)
{
	bool result{};
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	result = status == GL_TRUE;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &status);
	if (status && linkLog)
	{
		GLchar *logBuffer = new GLchar[status];
		glGetProgramInfoLog(program, status, nullptr, logBuffer);
		*linkLog = logBuffer;
		delete[] logBuffer;
	}
	return result;
}

ShaderProgram::ShaderProgram(std::shared_ptr<Shader> shader)
	:shader{shader}
{
	Link();
}

ShaderProgram::ShaderProgram(const ShaderProgram& other)
	:shader{other.shader}
{
	Link();
}

ShaderProgram::ShaderProgram(ShaderProgram&& other)
	:ShaderProgram()
{
	swap(*this, other);
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(program);
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other)
{
	swap(*this, other);
	return *this;
}

ShaderProgram& ShaderProgram::operator=(const ShaderProgram& other)
{
	ShaderProgram tmp{ other };
	swap(*this, tmp);
	return *this;
}

void ShaderProgram::Use() const
{
	glUseProgram(program);
}

GLuint ShaderProgram::GetProgramObject() const
{
	return program;
}

bool ShaderProgram::IsValid() const
{
	return program != 0;
}

void ShaderProgram::Link()
{
	program = glCreateProgram();
	for (const auto &shaderElement : (*shader))
	{
		glAttachShader(program, shaderElement->GetShaderObject());
	}
	glBindAttribLocation(program, VertexAttributeLocation::Position, "position");
	glBindAttribLocation(program, VertexAttributeLocation::Normal, "normal");
	glBindAttribLocation(program, VertexAttributeLocation::Tangent, "tangent");
	glBindAttribLocation(program, VertexAttributeLocation::Color, "color");
	glBindAttribLocation(program, VertexAttributeLocation::UV, "uv");
	std::cout << "Linking shader program...\n";
	glLinkProgram(program);
	std::string log;
	if (!DidProgramLink(program, &log))
		std::cout << "Link error!\n";
	std::cout << "Log:\n" << log << std::endl;
}

void swap(ShaderProgram& first, ShaderProgram& second)
{
	using std::swap;
	swap(first.program, second.program);
	swap(first.shader, second.shader);
}

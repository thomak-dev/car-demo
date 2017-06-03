#pragma once
#include <memory>
#include <vector>
#include <GL/glew.h>
#include "core.h"

class Shader;

class ShaderProgram
{
public:
	ShaderProgram(std::shared_ptr<Shader> shader);
	ShaderProgram(const ShaderProgram& other);
	ShaderProgram(ShaderProgram&& other);
	~ShaderProgram();
	ShaderProgram& operator=(ShaderProgram&& other);
	ShaderProgram& operator=(const ShaderProgram& other);

	void Use() const;
	GLuint GetProgramObject() const;
	bool IsValid() const;

	friend void swap(ShaderProgram& first, ShaderProgram& second);
private:
	ShaderProgram() = default;
	GLuint program{};
	std::shared_ptr<Shader> shader;

	void Link();
};


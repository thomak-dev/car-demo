#pragma once
#include <memory>
#include "static_glew.h"

class Shader;

class ShaderProgram
{
public:
	ShaderProgram(std::shared_ptr<Shader> shader);
	ShaderProgram(const ShaderProgram& other);
	ShaderProgram(ShaderProgram&& other) noexcept;
	~ShaderProgram();
	ShaderProgram& operator=(ShaderProgram&& other) noexcept;
	ShaderProgram& operator=(const ShaderProgram& other);

	void Use() const;
	GLuint GetProgramObject() const;
	bool IsValid() const;

	friend void swap(ShaderProgram& first, ShaderProgram& second) noexcept;
private:
	ShaderProgram() = default;
	GLuint program{};
	std::shared_ptr<Shader> shader;

	void Link();
};


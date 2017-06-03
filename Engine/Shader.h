#pragma once
#include <memory>
#include <vector>
#include "core.h"

class ShaderElement;

class Shader
{
	DELETE_COPY(Shader)
public:
	Shader() = default;
	Shader(Shader&&);

	Shader& operator=(Shader&&);

	void AddElement(const std::shared_ptr<ShaderElement>& element);
	auto begin() const { return shaderElements.begin(); }
	auto end() const { return shaderElements.end(); }

	friend void swap(Shader& first, Shader& second);
private:
	std::vector<std::shared_ptr<ShaderElement>> shaderElements;
};


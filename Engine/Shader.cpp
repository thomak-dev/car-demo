#include "Shader.h"

Shader::Shader(Shader&& other)
	:Shader{}
{
	swap(*this, other);
}

Shader& Shader::operator=(Shader&& other)
{
	swap(*this, other);
	return *this;
}

void Shader::AddElement(const std::shared_ptr<ShaderElement>& element)
{
	shaderElements.push_back(element);
}

void swap(Shader& first, Shader& second)
{
	using std::swap;
	swap(first.shaderElements, second.shaderElements);
}

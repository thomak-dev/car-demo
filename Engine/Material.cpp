#include "Material.h"
#include <glm/gtc/type_ptr.hpp>
#include "Renderer.h"

Material::Material(ShaderProgram&& shader)
	:program{std::move(shader)}
{
	UpdateUniformLocations();
}

Material::Material(const Material& other)
	:
	cull{ other.cull }, testDepth{other.testDepth}, writeDepth{other.writeDepth},
	blendOpColor{other.blendOpColor}, blendOpAlpha{other.blendOpAlpha},
	blendSrcColor{other.blendSrcColor}, blendSrcAlpha{other.blendSrcAlpha}, blendDstColor{other.blendDstColor}, blendDstAlpha{other.blendDstAlpha}, 
	ordinal{other.ordinal}, program{ other.program }, properties{ other.properties }
{
	for (auto& prop : properties)
	{
		prop.dirty = true;
	}
	UpdateUniformLocations();
}

Material& Material::operator=(const Material& material)
{
	Material tmp{ material };
	swap(*this, tmp);
	return *this;
}

void Material::SetShader(ShaderProgram&& shader)
{
	this->program = std::move(shader);
	UpdateUniformLocations();
}

void Material::Use()
{
	if (dirty)
		UpdateUniformLocations();
	program.Use();
}

const ShaderProgram& Material::GetShaderProgram() const
{
	return program;
}

void Material::UpdateUniformLocations()
{
	uniformLocations.clear();
	program.Use();
	GLuint programObject = program.GetProgramObject();
	uniformModel = glGetUniformLocation(programObject, "model");
	GLuint viewProj = glGetUniformBlockIndex(programObject, "ViewProj");
	GLuint lights = glGetUniformBlockIndex(programObject, "Lights");
	if (viewProj != GL_INVALID_INDEX)
		glUniformBlockBinding(programObject, viewProj, UniformBufferIndex::ViewProjection);
	if (lights != GL_INVALID_INDEX)
		glUniformBlockBinding(programObject, lights, UniformBufferIndex::Lights);
	int textureUnitOffset = 0;
	for (auto& property : properties)
	{
		if (property.type == Material::Property::Type::Texture2D)
		{
			GLint location = glGetUniformLocation(programObject, property.name.c_str());
			uniformLocations[property.name] = location;
			glUniform1i(location, textureUnitOffset);
			++textureUnitOffset;
		}
		else if (property.type == Material::Property::Type::Float)
		{
			GLint location = glGetUniformLocation(programObject, property.name.c_str());
			uniformLocations[property.name] = location;
			glUniform1f(location, property.GetFloat());
		}
		else if (property.type == Material::Property::Type::Vector4)
		{
			GLint location = glGetUniformLocation(programObject, property.name.c_str());
			uniformLocations[property.name] = location;
			glUniform4fv(location, 1, glm::value_ptr(property.GetVector4()));
		}
		property.dirty = false;
	}
	dirty = false;
}

Material::Property::Property(const std::string& name, float value)
	:name{ name }, type{ Type::Float }, floatValue{ value }
{
}

Material::Property::Property(const std::string& name, const glm::vec4& value)
	:name{ name }, type{ Type::Vector4 }, vector4Value{value}
{
}

Material::Property::Property(const std::string& name, const std::shared_ptr<Texture2D>& value)
	:name{ name }, type{ Type::Texture2D }, textureValue{ value }
{
}

Material::Property::Property(const Property& other)
	:name{other.name}, type{other.type}, dirty{other.dirty}
{
	switch (type)
	{
	case Type::Texture2D:
		new (&textureValue) std::shared_ptr<Texture2D>{other.textureValue};
		break;
	case Type::Float:
		floatValue = other.floatValue;
		break;
	case Type::Vector4:
		vector4Value = other.vector4Value;
		break;
	default:
		PRO_ASSERT(false);
		break;
	}
}

Material::Property::~Property()
{
	if (type == Type::Texture2D)
		textureValue.~shared_ptr();
}

void Material::Property::SetValue(float value)
{
	if (type == Type::Texture2D)
		textureValue.~shared_ptr();
	type = Type::Float;
	floatValue = value;
	dirty = true;
}

void Material::Property::SetValue(const std::shared_ptr<Texture2D>& value)
{
	type = Type::Texture2D;
	new (&textureValue) std::shared_ptr<Texture2D>{value};
	dirty = true;
}

void Material::Property::SetValue(const glm::vec4& value)
{
	if (type == Type::Texture2D)
		textureValue.~shared_ptr();
	type = Type::Vector4;
	vector4Value = value;
	dirty = true;
}

std::shared_ptr<Texture2D> Material::Property::GetTexture2D() const
{
	PRO_ASSERT(type == Type::Texture2D);
	return textureValue;
}

float Material::Property::GetFloat() const
{
	PRO_ASSERT(type == Type::Float);
	return floatValue;
}

glm::vec4 Material::Property::GetVector4() const
{
	PRO_ASSERT(type == Type::Vector4);
	return vector4Value;
}

void swap(Material& first, Material& second) noexcept
{
	using std::swap;
	swap(first.program, second.program);
	swap(first.properties, second.properties);
	swap(first.uniformModel, second.uniformModel);
	swap(first.uniformLocations, second.uniformLocations);
	swap(first.cull, second.cull);
	swap(first.testDepth, second.testDepth);
	swap(first.writeDepth, second.writeDepth);
	swap(first.blendOpColor, second.blendOpColor);
	swap(first.blendOpAlpha, second.blendOpAlpha);
	swap(first.blendSrcColor, second.blendSrcColor);
	swap(first.blendSrcAlpha, second.blendSrcAlpha);
	swap(first.blendDstColor, second.blendDstColor);
	swap(first.blendDstAlpha, second.blendDstAlpha);
	swap(first.ordinal, second.ordinal);
}

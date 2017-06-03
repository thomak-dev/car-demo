#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <SDL.h>
#include <glm/vec4.hpp>
#include "core.h"
#include "ShaderProgram.h"
#include "Renderer.h"

class Texture2D;

class Material
{
public:
	DELETE_MOVE(Material)
		struct Property
	{
		DELETE_MOVE(Property)
			enum class Type
		{
			Texture2D,
			Float,
			Vector4
		};

		Property(const std::string& name, float value);
		Property(const std::string& name, const glm::vec4& value);
		Property(const std::string& name, const std::shared_ptr<Texture2D>& value);
		Property(const Property& other);
		~Property();

		std::string name;
		Type type{ Type::Float };
		bool dirty{ true };

		void SetValue(float value);
		void SetValue(const std::shared_ptr<Texture2D>& value);
		void SetValue(const glm::vec4& value);

		std::shared_ptr<Texture2D> GetTexture2D() const;
		float GetFloat() const;
		glm::vec4 GetVector4() const;

	private:
		union
		{
			float floatValue{};
			std::shared_ptr<Texture2D> textureValue;
			glm::vec4 vector4Value;
		};
	};

	Material(ShaderProgram&&);
	Material(const Material& other);
	~Material() = default;

	Material& operator=(const Material& material);

	bool cull{true};
	bool testDepth{true};
	bool writeDepth{true};
	Blend::Operation::Type blendOpColor{ Blend::Operation::Add };
	Blend::Operation::Type blendOpAlpha{ Blend::Operation::Add };
	Blend::Coefficient::Type blendSrcColor{ Blend::Coefficient::One };
	Blend::Coefficient::Type blendSrcAlpha{ Blend::Coefficient::One };
	Blend::Coefficient::Type blendDstColor{ Blend::Coefficient::Zero };
	Blend::Coefficient::Type blendDstAlpha{ Blend::Coefficient::Zero };

	std::unordered_map<std::string, GLint> uniformLocations;
	GLint uniformModel{ -1 };

	// determines in which order things are drawn (ascending order)
	int ordinal{};

	void SetShader(ShaderProgram&& shader);
	const ShaderProgram& GetShaderProgram() const;
	std::vector<Property>& GetProperties() { return properties; };

	template <typename T>
	void SetProperty(const std::string& name, T value);
	friend void swap(Material& first, Material& second);
private:
	ShaderProgram program;
	std::vector<Property> properties;

	void UpdateUniformLocations();
};

template <typename T>
void Material::SetProperty(const std::string& name, T value)
{
	auto property = std::find_if(properties.begin(), properties.end(), [&](const auto& pr) { return pr.name == name; });
	if (property == properties.end())
		properties.emplace_back(name, value);
	else
		property->SetValue(value);
}
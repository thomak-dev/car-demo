#pragma once
#include <unordered_map>
#include <glm/vec4.hpp>
#include "Component.h"

class Light : public Component
{
	DELETE_COPY_MOVE(Light)
public:
	enum class Type
	{
		Directional,
		Point
	};

	explicit Light(Entity* entity);
	virtual ~Light();

	float Range() const { return range; }
	float Pointedness() const { return pointedness; }

	void SetRange(float range);
	void SetPointedness(float linearity);
	void SetCutoff(float cutoff);

	float LinearAttenuation() const { return linearAttenuation; }
	float SquareAttenuation() const { return squareAttenuation; }
	float Cutoff() const { return cutoff; }
	virtual void Deserialize(const Json& json) override;

	glm::vec4 color{ 1, 1, 1, 1 };
	Type type {Type::Directional};

private:
	float range{1.0f};
	float pointedness{0.5f}; // 1 = linear coefficient is higher, 0 = square coefficient is higher. 1 has a steep falloff at the start and gets less steep over time, 0 has a slowly increasing falloff, but eventually it decreases again as well
	float linearAttenuation;
	float squareAttenuation;
	float cutoff{ 1 / 32.0f };

	static std::unordered_map<std::string, Type> stringToType;

	void RecomputeAttenuation();
};


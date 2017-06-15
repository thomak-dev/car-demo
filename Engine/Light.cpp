#include "Light.h"
#include <SDL.h>
#include <glm/common.hpp>
#include "Renderer.h"

std::unordered_map<std::string, Light::Type> Light::stringToType
{
	{ "Directional", Type::Directional },
	{ "Point", Type::Point }
};

Light::Light(Entity& entity)
	:Component{entity}
{
	RecomputeAttenuation();
	Renderer::Instance().RegisterLight(this);
}

Light::~Light()
{
	Renderer::Instance().UnregisterLight(this);
}

void Light::SetRange(float range)
{
	this->range = range;
	RecomputeAttenuation();
}

void Light::SetPointedness(float linearity)
{
	SDL_assert(linearity >= 0 && linearity <= 1);
	this->pointedness = linearity;
	RecomputeAttenuation();
}

void Light::SetCutoff(float cutoff)
{
	SDL_assert(cutoff > 0 && cutoff < 1);
	this->cutoff = cutoff;
	RecomputeAttenuation();
}

void Light::Deserialize(const Json& json)
{
	if (json.HasMember("range"))
		SetRange(json["range"].GetFloat());
	if (json.HasMember("pointedness"))
		SetPointedness(json["pointedness"].GetFloat());
	if (json.HasMember("cutoff"))
		SetCutoff(json["cutoff"].GetFloat());
	if (json.HasMember("color"))
		color = ToVec4(json["color"]);
	if (json.HasMember("light_type"))
		type = stringToType[json["light_type"].GetString()];
}

//float Attenuation(float x, float lin, float sq, float cut)
//{
//	return((1 / (1 + lin * x + sq * x * x)) - cut) * (1 + cut);
//}

void Light::RecomputeAttenuation()
{
	float c = (1 - cutoff) / cutoff; // atten drops to cutoff at range
	float linearAttenMax = c / range;
	// coefficients for attenuation function: atten = ((1 / (1 + lin * x + sq * x * x)) - cut) * (1 + cut)
	linearAttenuation = pointedness * linearAttenMax;
	squareAttenuation = (c - linearAttenuation * range) / (range * range);
}

#include "Light.h"
#include <SDL.h>
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
	PRO_ASSERT(linearity >= 0 && linearity <= 1);
	this->pointedness = linearity;
	RecomputeAttenuation();
}

void Light::SetCutoff(float cutoff)
{
	PRO_ASSERT(cutoff > 0 && cutoff < 1);
	this->cutoff = cutoff;
	RecomputeAttenuation();
}

int Light::Deserialize(const Json& json)
{
	int count = 0;
	if (json.HasMember("range") && ++count)
		SetRange(json["range"].GetFloat());
	if (json.HasMember("pointedness") && ++count)
		SetPointedness(json["pointedness"].GetFloat());
	if (json.HasMember("cutoff") && ++count)
		SetCutoff(json["cutoff"].GetFloat());
	if (json.HasMember("color") && ++count)
		color = ToVec4(json["color"]);
	if (json.HasMember("light_type") && ++count)
		type = stringToType[json["light_type"].GetString()];
	return count;
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

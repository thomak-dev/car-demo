#version 150

#define LIGHT_COUNT 4

struct Light
{
	vec4 dirPos;
	vec4 color;
	float linearAtten;
	float squareAtten;
	float cutoff;
};


layout(std140) uniform Lights
{
	Light lights[LIGHT_COUNT];
};

vec3 UnpackNormal(vec3 mapNormal)
{
	if(length(mapNormal) == 0)
		return vec3(0, 0, 1);
	else
		return normalize(mapNormal * 2 - 1);
}

float Attenuation(float distance, float linear, float square, float cutoff)
{
	return clamp(((1 / (1 + linear * distance + square * distance * distance)) - cutoff) * (1 + cutoff), 0, 1);
}

vec3 ComputeLambert(vec3 pos, vec3 normal, vec3 albedo)
{
	vec3 col = vec3(0);
	for(int i = 0; i < LIGHT_COUNT; ++i)
	{
		vec3 lightDir = (lights[i].dirPos.w == 1? 1 : -1) * lights[i].dirPos.xyz - pos * lights[i].dirPos.w;
		col += (lights[i].dirPos.w == 1 ? Attenuation(length(lightDir), lights[i].linearAtten, lights[i].squareAtten, lights[i].cutoff) : 1) * albedo * clamp(dot(normal, normalize(lightDir)), 0, 1) * lights[i].color.rgb * lights[i].color.a;
	}
	return col;
}
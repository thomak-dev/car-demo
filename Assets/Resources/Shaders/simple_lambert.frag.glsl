#version 150

in vec3 worldPos;
in vec3 worldNormal;

uniform vec4 color;

out vec4 fragData;

vec3 ComputeLambert(vec3 pos, vec3 normal, vec3 albedo);

void main()
{
	fragData = vec4(ComputeLambert(worldPos, worldNormal, color.rgb), 1);
}
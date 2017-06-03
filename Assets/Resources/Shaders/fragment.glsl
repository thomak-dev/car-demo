#version 150

in vec3 worldPos;
in vec2 texCoords;
in mat3 tangentSpace;

uniform sampler2D colorMap;
uniform sampler2D normalMap;
uniform vec4 color;

out vec4 fragData;

vec3 UnpackNormal(vec3);
vec3 ComputeLambert(vec3 pos, vec3 normal, vec3 albedo);

void main()
{
	vec3 normal = tangentSpace * UnpackNormal(texture(normalMap, texCoords).rgb);
	vec3 albedo = texture(colorMap, texCoords).rgb * color.rgb;
	vec3 col = ComputeLambert(worldPos, normal, albedo);
	fragData = vec4(col, 1);
}
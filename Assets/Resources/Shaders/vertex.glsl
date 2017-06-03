#version 150

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec2 uv;

uniform mat4 model;

layout(std140) uniform ViewProj
{
	mat4 view;
	mat4 proj;
	vec3 viewPos;
};

out vec3 worldPos;
out vec2 texCoords;
out mat3 tangentSpace;

void main()
{
	vec3 worldNormal = normalize((inverse(transpose(model)) * vec4(normal, 0)).xyz);
	vec3 worldTangent = normalize(model * vec4(tangent, 0));
	tangentSpace = mat3(
		worldTangent,
		cross(worldNormal, worldTangent),
		worldNormal
	);
	texCoords = uv;
	vec4 worldPosH = model * vec4(position, 1);
	worldPos = worldPosH.xyz;
	gl_Position = proj * view * worldPosH;
}
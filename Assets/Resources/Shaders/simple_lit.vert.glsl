#version 150

in vec3 position;
in vec3 normal;

layout(std140) uniform ViewProj
{
	mat4 view;
	mat4 proj;
	vec3 viewPos;
};

uniform mat4 model;

out vec3 worldNormal;
out vec3 worldPos;

void main()
{
	worldNormal = normalize((inverse(transpose(model)) * vec4(normal, 0)).xyz);
	vec4 worldPosH = model * vec4(position, 1);
	worldPos = worldPosH.xyz;
	gl_Position = proj * view * worldPosH;
}
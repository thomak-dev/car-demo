#version 150

in vec3 position;
in vec2 uv;

layout(std140) uniform ViewProj
{
	mat4 view;
	mat4 proj;
	vec3 viewPos;
};

uniform mat4 model;

out vec2 texCoords;

void main()
{
	texCoords = uv;
	gl_Position = proj * view * model * vec4(position, 1);
}
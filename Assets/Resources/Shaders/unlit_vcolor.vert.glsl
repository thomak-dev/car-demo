#version 150

in vec3 position;
in vec4 color;

layout(std140) uniform ViewProj
{
	mat4 view;
	mat4 proj;
	vec3 viewPos;
};

out vec4 vColor;

void main()
{
	vColor = color;
	gl_Position = proj * view * vec4(position, 1);
}
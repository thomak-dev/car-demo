#version 150

in vec2 texCoords;

uniform sampler2D colorMap;
uniform vec4 color;

out vec4 fragData;

void main()
{
	fragData = texture(colorMap, texCoords) * color;
}
#version 430 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D screenTexture;
uniform vec2 screenSize;

void main()
{
	FragColor = texture(screenTexture, TexCoord);
}
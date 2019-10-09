#version 330 core

in vec2 colData;
out vec4 fragColor;

void main()
{
    fragColor = vec4(colData, 0.0f, 0.5f);
}
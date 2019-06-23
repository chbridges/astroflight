#version 330 core

in vec3 vtxColor;
out vec4 fragColor;

void main()
{
    fragColor = vec4(vtxColor, 1.0f);
}
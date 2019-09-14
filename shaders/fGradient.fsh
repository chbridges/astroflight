#version 330 core

out vec4 fragColor;

uniform vec3 color;
in vec2 vtxData;

void main()
{
    float alpha = 1.0f - length(vtxData);

    fragColor = vec4(color, alpha);
}
#version 330 core

out vec4 fragColor;

in vec2 vtxData;

void main()
{
    float alpha = -0.5f + length(vtxData) * length(vtxData);
    fragColor = vec4(100.0f / 255.0f, 190.0f / 255.0f, 210.0f / 255.0f, alpha);
}
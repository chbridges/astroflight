#version 330 core

out vec4 fragColor;

in vec2 vtxData;

void main()
{
    float alpha = 1.0f - length(vtxData);

    fragColor = vec4(0.0f, 0.0f, 1.0f, alpha);
}
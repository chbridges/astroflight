#version 330 core

out vec4 fragColor;

in vec2 vtxData;

void main()
{
    //float alpha = 1.0f - length(vtxData);

    float green = 1.0f - length(vtxData);
    float red = length(vtxData);

    fragColor = vec4(red, green, 0.0f, 0.2f);
}
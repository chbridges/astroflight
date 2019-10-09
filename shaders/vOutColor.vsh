#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aColor;

uniform mat4 model;
uniform mat4 projection;

out vec2 vtxData;
out vec2 colData;

void main()
{
    vtxData = aPos;
    colData = aColor;
    gl_Position = projection * model * vec4(aPos, 0.0f, 1.0f);
}
#version 330 core

layout (location = 0) in vec2 aPos;

uniform mat4 model;
uniform mat4 projection;

out vec3 vtxColor;

void main()
{
    gl_Position = projection * model * vec4(aPos, 0.0f, 1.0f);
    vtxColor = vec3(aPos, 0.0f);
}
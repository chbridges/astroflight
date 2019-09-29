#version 330 core

struct Light
{
    vec3 direction;
    vec3 color;
};

in vec2 vtxData;
out vec4 fragColor;

uniform Light light;
uniform vec3 color;

void main()
{
    vec3 fragPos = vec3(vtxData, 0.0f);
    vec3 normal = normalize(fragPos);   // only valid for round objects
    vec3 lightDir = normalize(-light.direction);

    float diffuse = max(dot(normal, lightDir), -0.5f);
    vec4 diffuseLighting = vec4(diffuse * light.color / 255.0f, 1.0f);

    vec4 mainColor = vec4(color / 255.0f, 1.0f);

    fragColor = mainColor + mainColor * diffuseLighting * pow(length(fragPos), 10.0f);
}
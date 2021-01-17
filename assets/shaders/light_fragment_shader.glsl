#version 410 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 MaskColor;

uniform vec3 color;

void main()
{
    FragColor = vec4(color*1.0f, 1.0);
    MaskColor = vec4(color*10.0f, 1.0);
}
// Fragment Shader
#version 410 core
precision mediump float;

uniform sampler2D heightMap;

in vec2 Frag_UV;
in vec4 Frag_Color;
layout (location = 0) out vec4 Out_Color;

void main()
{
    Out_Color = texture(heightMap, Frag_UV);
}
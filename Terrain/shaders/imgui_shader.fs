// Fragment Shader
#version 410 core
precision mediump float;

uniform sampler2D heightMap;
uniform vec4 channels;
uniform bool alphaOnly;

in vec2 Frag_UV;
in vec4 Frag_Color;
layout (location = 0) out vec4 Out_Color;

void main()
{

	if(alphaOnly){
		float alphaChannel = texture(heightMap, Frag_UV).a;
		Out_Color = vec4(alphaChannel,alphaChannel,alphaChannel,1);
	}else{
		Out_Color = texture(heightMap, Frag_UV) * channels;
	}
}
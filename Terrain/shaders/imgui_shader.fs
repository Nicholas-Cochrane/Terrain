// Fragment Shader
#version 420 core
precision mediump float;

uniform sampler2D heightMap;
uniform float uTexelSize;
uniform float heightScale; // (number of units (or maximum tiles) / texture size in meters) * maximum height of height map from 0
uniform vec4 channels;
uniform bool alphaOnly;
uniform bool showMap;

in vec2 Frag_UV;
in vec4 Frag_Color;
layout (location = 0) out vec4 Out_Color;

void main()
{
	if(showMap){
		vec3 lightDir = normalize(vec3(-0.5f, 0.5f, 0f));
	
		float left  = texture(heightMap, Frag_UV + vec2(-uTexelSize, 0.0)).r* heightScale * 2.0 - 1.0;
		float right = texture(heightMap, Frag_UV + vec2( uTexelSize, 0.0)).r* heightScale * 2.0 - 1.0;
		float up    = texture(heightMap, Frag_UV + vec2(0.0,  uTexelSize)).r* heightScale * 2.0 - 1.0;
		float down  = texture(heightMap, Frag_UV + vec2(0.0, -uTexelSize)).r* heightScale * 2.0 - 1.0;
		vec3 normal = normalize(vec3(left - right, 2.0, up-down));
		float slope = max(dot(normal, vec3(0.0f,1.0f,0.0f)), 0.0f); // slope from up
		float diff = max(dot(normal, lightDir), 0.0);
		
		float h = texture(heightMap, Frag_UV).r;
		float Height = h * heightScale;
		if(Height < 0.0){
			Out_Color = vec4(0.098f,0.137f,0.659f, 1.0f);
		}else if(Height < 8.0f){ 
			Out_Color = vec4(0.965,0.843,0.69, 1.0f); // sand
		}else if(Height < 2750){//2750m / 3997m
			//1158m/ 3997m = 0.29
			//(x-0.1829)/(0.29-0.1829)  blend from grass to tree from 731m to 1158m
			Out_Color = mix(vec4(0.384,0.451,0.255f, 1.0f), vec4(0.1647f, 0.1961f ,0.1569f, 1.0f),clamp((h-0.1829)/(0.29-0.1829), 0.0f, 1.0f));
			//col = vec4( 0.1647f, 0.1961f ,0.1569f, 1.0f); // trees
		}else if(Height < 3000.0f){
			Out_Color = vec4(0.659,0.631,0.569, 1.0f);//rock
		}else{
			Out_Color = vec4(0.8902f, 0.9059f, 0.9294f, 1.0f);//snow
		}
		
		Out_Color = Out_Color * vec4((vec3(0.8, 0.8, 0.8) + diff),1.0f);
	}else if(alphaOnly){
		float alphaChannel = texture(heightMap, Frag_UV).a;
		Out_Color = vec4(alphaChannel,alphaChannel,alphaChannel,1);
	}else{
		Out_Color = texture(heightMap, Frag_UV).rgba * channels + vec4(0,0,0,(1-channels.a)*1);
	}
}
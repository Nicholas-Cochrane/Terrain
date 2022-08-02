// fragment shader
#version 410 core

uniform sampler2D heightMap;  // the texture corresponding to our height map

in float Height; //Height from Evaluation Shader
in float HeightScale; // Normal vector from Evaluation Shader
in vec2 TexCoords;

out vec4 FragColor;

float uTexelSize = 1.0f / 2556.0f;
vec3 lightDir = normalize(vec3(0.0f, 0.5f, -0.5f));

void main()
{
	float h = (Height + 16.0)/93.0f;
	float left  = texture(heightMap, TexCoords + vec2(-uTexelSize, 0.0)).r * HeightScale * 2.0 - 1.0;
	float right = texture(heightMap, TexCoords + vec2( uTexelSize, 0.0)).r * HeightScale * 2.0 - 1.0;
	float up    = texture(heightMap, TexCoords + vec2(0.0,  uTexelSize)).r * HeightScale * 2.0 - 1.0;
	float down  = texture(heightMap, TexCoords + vec2(0.0, -uTexelSize)).r * HeightScale * 2.0 - 1.0;
	vec3 normal = normalize(vec3(down - up, 2.0, left - right));
	float slope = max(dot(normal, vec3(0.0f,1.0f,0.0f)), 0.0f); // slope from up
	
	vec4 col = vec4(0.7176f, 0.5922f, 0.4353f, 1.0); //vec4(h, h, h, 1.0);
	
	if(slope < 0.7854){ // >45 degrees 
		col = vec4(0.6627f, 0.6314f, 0.5725, 1.0f);//rock
	}else if(h < 0.1829f){ //731m / 3997m
		col = vec4(0.7176f, 0.5922f, 0.4353f, 1.0); // grass
	}else if(h < 0.6880f){//2750m / 3997m
		//1158m/ 3997m = 0.29
		//(x-0.1829)/(0.29-0.1829)  blend from grass to tree from 731m to 1158m
		col = mix(vec4(0.7176f, 0.5922f, 0.4353f, 1.0), vec4(0.1647f, 0.1961f ,0.1569f, 1.0f),clamp((h-0.1829)/(0.29-0.1829), 0.0f, 1.0f));
		//col = vec4( 0.1647f, 0.1961f ,0.1569f, 1.0f); // trees
	}else if(h < 0.8694f){//3475m / 3997m
		col = vec4(0.6627f, 0.6314f, 0.5725, 1.0f);//rock
	}else{
		col = vec4(0.8902f, 0.9059f, 0.9294f, 1.0f);//snow
	}
	
	float diff = max(dot(normal, lightDir), 0.0);
	
	FragColor = vec4((vec3(0.207f, 0.318f, 0.361f) + diff) * col.rgb,1.0f);
}
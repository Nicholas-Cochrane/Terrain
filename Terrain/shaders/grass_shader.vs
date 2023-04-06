#version 420 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec2 aOffset;

out vec3 fpos;

uniform sampler2D heightMap;
uniform float heightScale;
uniform float worldSize;
uniform float uTexelSize;
uniform vec3 playerPos; 
uniform mat4 pvMatrix;
uniform float farDist;
uniform bool LODdist;

vec2 hash(vec2 p ){ 
	p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
	p = -1.0 + 2.0*fract(sin(p)*43758.5453123);
	return sin(p*6.283);
}


vec3 rotateAroundY(vec3 vertex, float angle){
	mat2 m = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
	vec2 result = m * vertex.xz;
	return vec3(result.x, vertex.y, result.y);
}

void main()
{
	float corner = 0;
	if (aOffset.x < -farDist+1.0 && aOffset.y < -farDist+1.0){
		corner = 1;
	}
	fpos = vec3(aOffset.x,corner,aOffset.y);
	if(LODdist){
		//fpos = vec3(0,1,0);
	}
	if(gl_InstanceID < 2u){
		fpos = vec3(1,1,1);
	}
	
	//wrap grass positions around
	float modOffsetX = floor((playerPos.x)/(farDist*2))*(farDist*2);
	if(abs((modOffsetX + aOffset.x)-playerPos.x) > farDist){
		modOffsetX = modOffsetX + farDist*2;
	}
	float modOffsetZ = floor((playerPos.z)/(farDist*2))*(farDist*2);
	if(abs((modOffsetZ + aOffset.y)-playerPos.z) > farDist){
		modOffsetZ = modOffsetZ + farDist*2;
	}
	
	vec2 combOffset = aOffset + vec2(modOffsetX,modOffsetZ);
	vec2 fadeDist = combOffset - playerPos.xz;
	float fadeStrength = max(abs(fadeDist.x), abs(fadeDist.y))/(farDist);
	fadeStrength *= distance(combOffset, playerPos.xz)/farDist;
	fadeStrength *= fadeStrength* fadeStrength * fadeStrength;
	fadeStrength -= 0.15;
	fadeStrength = max(0, fadeStrength);
	//fpos = vec3(fadeStrength,0,1);
	vec2 texCoord = ((worldSize/2) + vec2(combOffset.x, -combOffset.y))/worldSize;
	float height = texture(heightMap, texCoord).r * heightScale;
	float left  = texture(heightMap, texCoord + vec2(-uTexelSize, 0.0)).r * heightScale * 2.0 - 1.0;
	float right = texture(heightMap, texCoord + vec2( uTexelSize, 0.0)).r * heightScale * 2.0 - 1.0;
	float up    = texture(heightMap, texCoord + vec2(0.0,  uTexelSize)).r * heightScale * 2.0 - 1.0;
	float down  = texture(heightMap, texCoord + vec2(0.0, -uTexelSize)).r * heightScale * 2.0 - 1.0;
	vec3 normal = normalize(vec3(left - right, 2.0, up-down));
	float slope = max(dot(normal, vec3(0.0f,1.0f,0.0f)), 0.0f); // slope from up
	if(slope < radians(24.0) || height < 8.0f){
		gl_Position = vec4(-2,-2,-2,1);// do not render
		return;
	}
	
	vec3 rotatedPos = rotateAroundY(apos, hash(aOffset).x);
    gl_Position = pvMatrix * vec4(rotatedPos + vec3(combOffset.x, height-fadeStrength, combOffset.y), 1.0);
}

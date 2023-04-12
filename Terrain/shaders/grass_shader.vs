#version 420 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec2 aOffset;

out vec3 fpos;
out vec3 vertNormal;

uniform sampler2D heightMap;
uniform sampler2D windMap;
uniform float heightScale;
uniform float worldSize;
uniform float uTexelSize;
uniform vec3 playerPos; 
uniform mat4 pvMatrix;
uniform float farDist;
uniform bool LODdist;
uniform float time;
uniform float windAngle;

#define PI 3.1415926535897932384626433832795

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
	fpos = apos;
	vec3 startingNorm = normalize(vec3(0,0,1));

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

	vec2 texCoord = ((worldSize/2) + vec2(combOffset.x, -combOffset.y))/worldSize;
	float height = texture(heightMap, texCoord).r * heightScale;
	float left  = texture(heightMap, texCoord + vec2(-uTexelSize, 0.0)).r * heightScale * 2.0 - 1.0;
	float right = texture(heightMap, texCoord + vec2( uTexelSize, 0.0)).r * heightScale * 2.0 - 1.0;
	float up    = texture(heightMap, texCoord + vec2(0.0,  uTexelSize)).r * heightScale * 2.0 - 1.0;
	float down  = texture(heightMap, texCoord + vec2(0.0, -uTexelSize)).r * heightScale * 2.0 - 1.0;
	vec3 groundNormal = normalize(vec3(left - right, 2.0, up-down));
	float slope = max(dot(groundNormal, vec3(0.0f,1.0f,0.0f)), 0.0f); // slope from up
	if(slope < radians(24.0) || height < 8.0f){
		gl_Position = vec4(-2,-2,-2,1);// do not render
		return;
	}
	float windRadians = (windAngle)*(PI/180);
	vec2 windDirection = vec2(sin(windRadians), -cos(windRadians));
	vec2 offsetHash = hash(aOffset);
	float windSpeed = 3;
	float scrollSpeed = 0.03;
	
	float windStrength = texture(windMap, mat2(cos(windRadians), -sin(windRadians), sin(windRadians), cos(windRadians))*(windDirection*vec2(time,time)*scrollSpeed + mod(offsetHash, 0.03)+ (worldSize/200.0)*texCoord)).r;
	
	/*
	float corner = 0;
	if (aOffset.x < -farDist+1.0 && aOffset.y < -farDist+1.0){
		corner = 1;
	}
	fpos = vec3(windStrength,corner,aOffset.y);
	*/
	
	/*if(gl_InstanceID < 2u){// find corners of chunk
		fpos = vec3(1,1,1); 
	}*/
	
	float heightPower = pow(apos.y,1.5);
	
	vec3 rotatedPos = apos;
	vertNormal = startingNorm;
	
	//vec3 rotatedPos = rotateAroundY(apos, offsetHash.x);
	//vertNormal = rotateAroundY(startingNorm,offsetHash.x);              //-0.1                       /0.15
	float zWindOffset = (heightPower * windSpeed *(pow(windStrength,1.3)-0.0)*0.4* windDirection.y) + 0.0 * heightPower * windDirection.y;
	float xwindOffset = (heightPower * windSpeed * (pow(windStrength,1.3)-0.0)*0.4* -windDirection.x) + 0.0 * heightPower * -windDirection.x;
	
	vec3 adjacentPointA = rotatedPos + rotateAroundY(vertNormal, (PI/2));
	adjacentPointA += vec3(xwindOffset, 0, zWindOffset);
	
	vec3 adjacentPointB = rotatedPos + vec3(0,0.05,0);
	float pointBHeightPower =  pow(adjacentPointB.y,1.5);
	adjacentPointB.z += (pointBHeightPower * windSpeed *(pow(windStrength,1.3)-0.0)*0.4* windDirection.y) + 0.0 * pointBHeightPower * windDirection.y;
	adjacentPointB.x = (pointBHeightPower * windSpeed * (pow(windStrength,1.3)-0.0)*0.4* -windDirection.x) + 0.0 * pointBHeightPower * -windDirection.x;
	
	rotatedPos += vec3(xwindOffset, 0, zWindOffset); // no longer using rotatedPos to create offset points
	vertNormal = normalize(cross(rotatedPos-adjacentPointA, rotatedPos-adjacentPointB));
	
	
    gl_Position = pvMatrix * vec4(rotatedPos + vec3(combOffset.x, height-fadeStrength, combOffset.y), 1.0);
}

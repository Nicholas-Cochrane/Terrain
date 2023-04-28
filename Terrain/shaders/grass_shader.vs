#version 420 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec2 aOffset;

layout (std140, binding = 0) uniform lowUpdateShared
{
	float heightScale; // height of 1.0 on height map (aka heightest possible point on map)
	float worldSize; // size of map in units/meters
	float uTexelSize; // texel size of hieghtmap or 1.0/(height map resolution)
	float nearPlane;
	float farPlane;
};

layout (std140, binding = 1) uniform freqUpdateShared
{
	float time;
	float windAngle;
	vec4 sunDirection;
	vec4 playerPos;
};

out vec3 fpos;
out vec4 offsetPos;
out vec3 vertNormal;
out vec3 gNormal;
out float grassHeight;
//out vec3 debugColor;
//out float corner;

uniform sampler2D heightMap;
uniform sampler2D windMap;
uniform sampler2D grassHeightMap;
uniform mat4 pvMatrix;
uniform float farDist;
uniform bool LODdist;

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

	vec2 texCoord = ((worldSize/2) + vec2(combOffset.x, -combOffset.y))/worldSize;
	float height = texture(heightMap, texCoord).r * heightScale;
	float left  = texture(heightMap, texCoord + vec2(-uTexelSize, 0.0)).r * heightScale * 2.0 - 1.0;
	float right = texture(heightMap, texCoord + vec2( uTexelSize, 0.0)).r * heightScale * 2.0 - 1.0;
	float up    = texture(heightMap, texCoord + vec2(0.0,  uTexelSize)).r * heightScale * 2.0 - 1.0;
	float down  = texture(heightMap, texCoord + vec2(0.0, -uTexelSize)).r * heightScale * 2.0 - 1.0;
	vec3 groundNormal = normalize(vec3(left - right, 2.0, up-down));
	gNormal = groundNormal;
	float slope = max(dot(groundNormal, vec3(0.0f,1.0f,0.0f)), 0.0f); // slope from up
	if(slope < radians(24.0) || height < 8.0f){
		gl_Position = vec4(-2,-2,-2,1);// do not render
		return;
	}
	
	float flyingFadeStrength = distance(vec3(combOffset.x, height, combOffset.y), playerPos.xyz)/(2*farDist);
	flyingFadeStrength = pow(flyingFadeStrength,10);
	vec2 fadeDist = combOffset - playerPos.xz;
	float fadeStrength = max(abs(fadeDist.x), abs(fadeDist.y))/(farDist);
	fadeStrength *= distance(combOffset, playerPos.xz)/farDist;
	fadeStrength *= fadeStrength* fadeStrength * fadeStrength;
	fadeStrength -= 0.15;
	fadeStrength = max(0, fadeStrength);
	fadeStrength = max(flyingFadeStrength, fadeStrength);
	
	
	float windRadians = (windAngle)*(PI/180);
	vec2 windDirection = vec2(sin(windRadians), -cos(windRadians));
	vec2 offsetHash = hash(aOffset);
	float windSpeed = 2;
	float scrollSpeed = 0.03;
	float windTextureSize = 200.0; //number of units 1 tile of texture covers
	float grassHeightTextureSize = 200.0; //number of units 1 tile of texture covers
	
	float windStrength = texture(windMap, mat2(cos(windRadians), -sin(windRadians), sin(windRadians), cos(windRadians))*(windDirection*vec2(time,time)*scrollSpeed + mod(offsetHash, 0.03)+ (worldSize/windTextureSize)*texCoord)).r;
	//vary height by perlin noise map
	vec3 heightAdjPos = apos;
	grassHeight = texture(grassHeightMap, (worldSize/grassHeightTextureSize)*texCoord).r;
	heightAdjPos.y *= grassHeight*1.0 + 0.3;
	heightAdjPos.xz *= sqrt(clamp(grassHeight+0.4,0,1)); // make shorter grass non linearly less wide
	fpos = heightAdjPos;
	
	/*
	float corner = 0;
	if (aOffset.x < -farDist+1.0 && aOffset.y < -farDist+1.0){
		corner = 1;
	}
	fpos = vec3(windStrength,corner,aOffset.y);
	*/
	
	/*if(gl_InstanceID < 2u){// find corners of chunk
		corner = 1.0; 
	}else{
		corner = 0.0;
	}*/
	
	float heightPower = pow(heightAdjPos.y,1.5);
	
	vec3 rotatedPos = rotateAroundY(heightAdjPos, offsetHash.x);
	vertNormal = rotateAroundY(startingNorm,offsetHash.x);              //-0.1                       /0.15
	float zWindOffset = (heightPower * windSpeed *(pow(windStrength,1.3)-0.1)*0.4* windDirection.y) + 0.15 * heightPower * windDirection.y;
	float xwindOffset = (heightPower * windSpeed * (pow(windStrength,1.3)-0.1)*0.4* -windDirection.x) + 0.15 * heightPower * -windDirection.x;
	
	vec3 adjacentPointA = rotatedPos + rotateAroundY(vertNormal, (PI/2));
	adjacentPointA += vec3(xwindOffset, 0, zWindOffset);
	
	vec3 adjacentPointB = rotatedPos + vec3(0,0.05,0);
	float pointBHeightPower =  pow(adjacentPointB.y,1.5);
	adjacentPointB.z += (pointBHeightPower * windSpeed *(pow(windStrength,1.3)-0.1)*0.4* windDirection.y) + 0.15 * pointBHeightPower * windDirection.y;
	adjacentPointB.x += (pointBHeightPower * windSpeed * (pow(windStrength,1.3)-0.1)*0.4* -windDirection.x) + 0.15 * pointBHeightPower * -windDirection.x;
	
	rotatedPos += vec3(xwindOffset, 0, zWindOffset); // no longer using rotatedPos to create offset points
	vertNormal = normalize(cross(rotatedPos-adjacentPointA, rotatedPos-adjacentPointB));
	
	
	offsetPos = vec4(rotatedPos + vec3(combOffset.x, height-fadeStrength, combOffset.y), 1.0);
    gl_Position = pvMatrix * vec4(rotatedPos + vec3(combOffset.x, height-fadeStrength, combOffset.y), 1.0);
}

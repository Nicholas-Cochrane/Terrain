#version 330 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec2 aOffset;

out vec3 fpos;

uniform sampler2D heightMap;
uniform float heightScale;
uniform float worldSize;
uniform float uTexelSize;
uniform vec3 playerPos; 
uniform mat4 pvMatrix;

void main()
{
	fpos = apos;
	vec2 combOffset = aOffset + playerPos.xz;
	vec2 texCoord = ((worldSize/2) + vec2(combOffset.x, -combOffset.y))/worldSize;
	float height = texture(heightMap, texCoord).r * heightScale;
	float left  = texture(heightMap, texCoord + vec2(-uTexelSize, 0.0)).r * heightScale * 2.0 - 1.0;
	float right = texture(heightMap, texCoord + vec2( uTexelSize, 0.0)).r * heightScale * 2.0 - 1.0;
	float up    = texture(heightMap, texCoord + vec2(0.0,  uTexelSize)).r * heightScale * 2.0 - 1.0;
	float down  = texture(heightMap, texCoord + vec2(0.0, -uTexelSize)).r * heightScale * 2.0 - 1.0;
	vec3 normal = normalize(vec3(left - right, 2.0, up-down));
	float slope = max(dot(normal, vec3(0.0f,1.0f,0.0f)), 0.0f); // slope from up
	if(slope < radians(24.0) || height < 8.0f){
		gl_Position = vec4(-2,-2,-2,1);
		return;
	}
    gl_Position = pvMatrix * vec4(apos + vec3(combOffset.x, height, combOffset.y), 1.0);
}

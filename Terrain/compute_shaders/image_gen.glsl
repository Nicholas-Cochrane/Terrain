#version 430 core
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(RG32F, binding = 0) uniform image2D imgOutput;
layout (location = 0) uniform int seed;
uniform uvec2 texRes;
uniform uint texHeight;
uniform uint texWidth;

const uint MAX_UINT = 4294967295;

uvec3 pcg3d(uvec3 v){
	//Mark Jarzynski and Marc Olano, Hash Functions for GPU Rendering, Journal of Computer Graphics Techniques (JCGT), vol. 9, no. 3, 21-38, 2020
	//Available online http://jcgt.org/published/0009/03/02/
	//JCGT ISSN 2331-7418
	v = v * 1664525u + 1013904223u;
	v.x += v.y*v.z; v.y += v.z*v.x; v.z += v.x*v.y;
	v ^= v >> 16u;
	v.x += v.y*v.z; v.y += v.z*v.x; v.z += v.x*v.y;
	return v;
}

float fnoise2d(vec2 uv){
	uvec3 input = uvec3(floatBitsToUint(uv),seed); 
	return float(pcg3d(input).r)/MAX_UINT;
}

float smoothNoise(vec2 uv){
	vec2 localUV = fract(uv);
	vec2 localID = floor(uv);
	
	localUV = localUV * localUV * (3.0-2.0*localUV); // quick smooth step
	
	float bl = fnoise2d(localID);
	float br = fnoise2d(localID + vec2(1,0));
	float bottom = mix(bl, br, localUV.x);
	
	float tl = fnoise2d(localID + vec2(0,1));
	float tr = fnoise2d(localID + vec2(1,1));
	float top = mix(tl, tr, localUV.x);
	
	return mix(bottom, top, localUV.y);
}

float valueNoise(vec2 uv){
	//Octaves
	float noiseValue = smoothNoise(uv*4.0);
	noiseValue += smoothNoise(uv*8.0)*0.5;
	noiseValue += smoothNoise(uv*16.0)*0.25;
	noiseValue += smoothNoise(uv*32.0)*0.125;
	noiseValue += smoothNoise(uv*64.0)*0.0625;
	noiseValue += smoothNoise(uv*128.0)*0.03125;
	noiseValue += smoothNoise(uv*256.0)*0.015625;
	noiseValue /= 1.984375;//Normalize
	return noiseValue;
}
	
void main() {
	vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	ivec2 origCoord = texelCoord - ivec2(texRes.x/2,texRes.y/2);
	vec2 uv = vec2(float(texelCoord.x)/texRes.x,float(texelCoord.y)/texRes.y);
	float r = float(pcg3d(uvec3(uint(texelCoord.x),uint(texelCoord.y),seed)).r)/MAX_UINT;
	
	
	//mask
    float dist = max(0,sqrt(pow((float(origCoord.x)/texRes.x)*2,2)+ pow((float(origCoord.y)/texRes.y)*2,2))-0.2)*1.2; // dist
    float euclidean = max(0,(pow((float(origCoord.x)/texRes.x)*3.35,2)+ pow((float(origCoord.y)/texRes.y)*3.35,2))/sqrt(2)); //Euclidean
	float sqrBump = max(0,(pow((float(origCoord.x)/texRes.x)*2,2)) * (pow((float(origCoord.y)/texRes.y)*2,2))); // square bump
	
	
	value.g = dist;
	value.r = valueNoise(uv) - 0.5 * 2.0 * dist;
	//value.rg = fract(uv*10);
    imageStore(imgOutput, texelCoord, value);
}
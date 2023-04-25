#version 430 core
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(R8, binding = 0) uniform image2D imgOutput;
layout (location = 0) uniform int seed;
uniform uvec2 texRes;

#define PI 3.1415926535897932384626433832795
#define MAX_UINT 4294967295u
#define NUM_OCTAVES 6

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
	uvec3 input = uvec3(floatBitsToInt(uv),seed);
	return ((float(pcg3d(input).r)/MAX_UINT)-0.5) * 2;
}

vec2 f2noise2d(vec2 uv){
	uvec3 input = uvec3(floatBitsToInt(uv),seed); 
	uvec3 value = pcg3d(input);
	return (vec2(float(value.r),float(value.g))/uvec2(MAX_UINT,MAX_UINT)-0.5) * 2;
}

vec2 f2noise2di(uvec2 uv){
	uvec3 value = pcg3d(uvec3(uv,seed));
	return (vec2(float(value.r),float(value.g))/uvec2(MAX_UINT,MAX_UINT)-0.5) * 2;
}

float repeatingPerlinNoise(vec2 p, float a){
	p *= a;
	vec2 localID = floor(p);
    vec2 localUV = fract(p);
    
    //vec2 weight = localUV*localUV*(3.0-2.0*localUV); //smoothstep
	vec2 weight = localUV*localUV*localUV*(localUV*(localUV*6.0-15.0)+10.0); // qunitic
    
    float f00 = dot(f2noise2d(mod(localID+vec2(0.0,0.0), a)),localUV-vec2(0.0,0.0));
    float f01 = dot(f2noise2d(mod(localID+vec2(0.0,1.0), a)),localUV-vec2(0.0,1.0));
    float f10 = dot(f2noise2d(mod(localID+vec2(1.0,0.0), a)),localUV-vec2(1.0,0.0));
    float f11 = dot(f2noise2d(mod(localID+vec2(1.0,1.0), a)),localUV-vec2(1.0,1.0));
    
    float top = mix(f00,f10,weight.x);
    float bottom = mix(f01,f11,weight.x);
    return mix(top,bottom,weight.y)*1.41;// 1.41 to "normalize" output to [-1,1]
}


float repeatingOctPNoiseLoop(vec2 uv, float aMul, float octaves){
	// recomened 8 to 10 octaves
    float a = 4.0 * aMul;
	float b = 1.0;
	float normalizer = 0;
	float noiseValue = 0.0;
	
	for(int i = 0; i < octaves; i++){
		noiseValue += repeatingPerlinNoise(uv,a)*b;
		normalizer += b;
		a *= 2.0;
		b *= 0.5;
	}
	noiseValue /= normalizer;
	return noiseValue;
}
	
void main() {
	vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	ivec2 origCoord = texelCoord - ivec2(texRes.x/2,texRes.y/2);
	vec2 uv = vec2(float(texelCoord.x)/texRes.x,float(texelCoord.y)/texRes.y);
	vec2 flippeduv = (-uv+1.0);
	
	value.r = ((repeatingOctPNoiseLoop(uv,0.5, 2))+ 1)/2;
	value.r = value.r*value.r*(3.0-2.0*value.r); // smoothstep
	//value.r = uv.x;
	float tufts = (repeatingOctPNoiseLoop(uv,10, 1));
	tufts = clamp(pow(tufts+0.3, 4), 0,1)+0.1 ;
	//value.r = mod(texelCoord.x,2);
	if(tufts > 0.9){
		//value.r += tufts;
	}else{
		//value.r += 0;
	}
	value.r += tufts;
	
    imageStore(imgOutput, texelCoord, value);
}
#version 430 core
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(RG32F, binding = 0) uniform image2D imgOutput;
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

vec2 hash(vec2 p ){ // relies on artifacts and may give different on different systems 
	p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
	p = -1.0 + 2.0*fract(sin(p)*43758.5453123);
	return sin(p*6.283 + seed);
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

float smoothNoise(vec2 uv){
	vec2 localUV = fract(uv);
	vec2 localID = floor(uv);
	
	localUV = localUV * localUV * (3.0-2.0*localUV); // quick smooth step
	//localUV = localUV*localUV*localUV*(localUV*(localUV*6.0-15.0)+10.0); // quick qunitic
	
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

float octVNoiseLoop(vec2 uv, float octaves){
	// recomened 8 to 11 octaves
    float a = 4.0;
	float b = 1.0;
	float normalizer = 0;
	float noiseValue = 0.0;
	
	for(int i = 0; i < octaves; i++){
		noiseValue += smoothNoise(uv*a)*b;
		normalizer += b;
		a *= 2.0;
		b *= 0.5;
	}
	noiseValue /= normalizer;
	return noiseValue;
}

float perlinNoise(vec2 p){
	vec2 localID = floor(p);
    vec2 localUV = fract(p);
    
    //vec2 weight = localUV*localUV*(3.0-2.0*localUV); //smoothstep
	vec2 weight = localUV*localUV*localUV*(localUV*(localUV*6.0-15.0)+10.0); // qunitic
    
    float f00 = dot(f2noise2d(localID+vec2(0.0,0.0)),localUV-vec2(0.0,0.0));
    float f01 = dot(f2noise2d(localID+vec2(0.0,1.0)),localUV-vec2(0.0,1.0));
    float f10 = dot(f2noise2d(localID+vec2(1.0,0.0)),localUV-vec2(1.0,0.0));
    float f11 = dot(f2noise2d(localID+vec2(1.0,1.0)),localUV-vec2(1.0,1.0));
    
    float top = mix(f00,f10,weight.x);
    float bottom = mix(f01,f11,weight.x);
    return mix(top,bottom,weight.y)*1.41;// 1.41 to "normalize" output to [-1,1]
}

float octPNoise(vec2 uv){
	float noiseValue = perlinNoise(uv*4.0);
	noiseValue += perlinNoise(uv*8.0)*0.5;
	noiseValue += perlinNoise(uv*16.0)*0.25;
	noiseValue += perlinNoise(uv*32.0)*0.125;
	noiseValue += perlinNoise(uv*64.0)*0.0625;
	noiseValue += perlinNoise(uv*128.0)*0.03125;
	noiseValue += perlinNoise(uv*256.0)*0.015625;
	noiseValue /= 1.984375;//Normalize
	return noiseValue;
}

float octPNoiseLoop(vec2 uv, float octaves){
	// recomened 8 to 10 octaves
    float a = 4.0;
	float b = 1.0;
	float normalizer = 0;
	float noiseValue = 0.0;
	
	for(int i = 0; i < octaves; i++){
		noiseValue += perlinNoise(uv*a)*b;
		normalizer += b;
		a *= 2.0;
		b *= 0.5;
	}
	noiseValue /= normalizer;
	return noiseValue;
}


float sdSegment( in vec2 p, in vec2 a, in vec2 b ){
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

vec2 smin( float a, float b, float k ){
    float h = max( k-abs(a-b), 0.0 )/k;
    float m = h*h*0.5;
    float s = m*k*(1.0/2.0);
    return (a<b) ? vec2(a-s,m) : vec2(b-s,1.0-m);
}

float smax1(float a, float b, float k){
    return log(exp(k * a) + exp(k * b)) / k;
}

float lineDomainWarp(vec2 uv){
	vec2 a = f2noise2di(uvec2(seed+1,seed+2));
	a.y *= sign(-a.x) + sign(-a.y); // if both are positive flip y coord sign
	float line = sdSegment(uv,vec2(0.0,0.0),a);
	return line;
}
	
void main() {
	vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	ivec2 origCoord = texelCoord - ivec2(texRes.x/2,texRes.y/2);
	vec2 uv = vec2(float(texelCoord.x)/texRes.x,float(texelCoord.y)/texRes.y);
	vec2 flippeduv = (-uv+1.0);
	bool randBool = bool(ceil(f2noise2di(uvec2(seed+1,seed+2)).y));
	float r = float(pcg3d(uvec3(uint(texelCoord.x),uint(texelCoord.y),seed)).r)/MAX_UINT;
	float noise = 0;
	
	
	//mask
    float dist = max(0,sqrt(pow((float(origCoord.x)/texRes.x)*2,2)+ pow((float(origCoord.y)/texRes.y)*2,2))-0.2)*1.2; // dist
    float euclidean = max(0,(pow((float(origCoord.x)/texRes.x)*3.35,2)+ pow((float(origCoord.y)/texRes.y)*3.35,2))/sqrt(2)); //Euclidean
	float sqrBump = max(0,(pow((float(origCoord.x)/texRes.x)*2,2)) * (pow((float(origCoord.y)/texRes.y)*2,2))); // square bump
		  sqrBump = max(0,(1.0-pow((float(origCoord.x)/texRes.x)*2,2)) * (1.0-pow((float(origCoord.y)/texRes.y)*2,2)));
	
	//for underwater shader ideas
	//vec2 q = vec2(perlinNoise(uv*2), perlinNoise(uv*2 + vec2(1.0,1.0)));
	//noise = octPNoise(uv+dist*0.8);
	
	//for range
	//noise = octVNoiseLoop(uv+perlinNoise(uv*64)*0.003, 8) + octPNoiseLoop(uv*128.0, 3)*0.00252;
	//noise = (max(0.005,abs(octVNoiseLoop(uv*1.2, 8))));
	
	//for normal islands
	noise = octPNoiseLoop(uv*0.5, 9)*1.2;
	
	//for fjord like islands
	//vec2 randuv = randBool? -uv+1.0 : uv;
	//noise = octPNoiseLoop((randuv*0.7)+lineDomainWarp(randuv*0.7), 9);
	
	if (noise < 0.2 && noise > -0.2){
		noise = sign(noise) * noise * noise * 5;//TODO Replace with a spline that ends at corect/better slope.
												// or replace with some post processing
	}
	//value.r = noise - smoothstep(0.6,0.99,dist)*0.4;
	value.r = mix(noise,log(noise+2.16395)-1.15182,smoothstep(0.6,0.99,dist)); // lerp between noise and noise from 0 to -1 on a ln curve
	value.g = 1.0;//octPNoiseLoop(uv*0.5, 9)*1.2;

	//value.rg = fract(uv*10);
    imageStore(imgOutput, texelCoord, value);
}
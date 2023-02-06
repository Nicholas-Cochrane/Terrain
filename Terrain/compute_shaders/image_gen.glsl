#version 430 core
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(RG32F, binding = 0) uniform image2D imgOutput;
layout (location = 0) uniform int seed;
uniform uvec2 texRes;
uniform uint texHeight;
uniform uint texWidth;

void main() {
	vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	ivec2 origCoord = texelCoord - ivec2(texRes.x/2,texRes.y/2);
	
    //float dist = 1.0-sqrt(pow((float(origCoord.x)/texRes.x)*2,2)+ pow((float(origCoord.y)/texRes.y)*2,2)); // dist
    float euclidean = max(-1.0,1.0-(pow((float(origCoord.x)/texRes.x)*2.35,2)+ pow((float(origCoord.y)/texRes.y)*2.35,2))/sqrt(2)); //Euclidean
	float sqrBump = max(-1.0,(1.0-pow((float(origCoord.x)/texRes.x)*2,2)) * (1.0-pow((float(origCoord.y)/texRes.y)*2,2))); // square bump
	value.g = sqrBump;
	value.r = max(sqrBump, euclidean);
	
    imageStore(imgOutput, texelCoord, value);
}
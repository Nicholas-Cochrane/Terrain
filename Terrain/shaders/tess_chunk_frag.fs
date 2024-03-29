// fragment shader
#version 420 core

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
	float time;         //0
	float windAngle;    //4
	vec4 sunDirection;  //16
	vec4 playerPos;     //32
};

uniform sampler2D heightMap;  // the texture corresponding to our height map
uniform float latitude;

in float Height; //Height from Evaluation Shader
in vec2 HeightMapCoords;
//in vec4 mpvResult;

out vec4 FragColor;


float LinearizeDepth(float depth) 
{
    // new 0 to 1 linerized depth value	
	return nearPlane * farPlane / (farPlane + depth * (nearPlane - farPlane));
}

vec4 cubic(float v){
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

vec4 textureBicubic(sampler2D sampler, vec2 texCoords){

   vec2 texSize = textureSize(sampler, 0);
   vec2 invTexSize = 1.0 / texSize;
   
   texCoords = texCoords * texSize - 0.5;

   
    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2 (-0.5, +1.5).xyxy;
    
    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;
    
    offset *= invTexSize.xxyy;
    
    vec4 sample0 = texture(sampler, offset.xz);
    vec4 sample1 = texture(sampler, offset.yz);
    vec4 sample2 = texture(sampler, offset.xw);
    vec4 sample3 = texture(sampler, offset.yw);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(
       mix(sample3, sample2, sx), mix(sample1, sample0, sx)
    , sy);
}

void main()
{
	float h = (Height)/heightScale;
	float left  = texture(heightMap, HeightMapCoords + vec2(-uTexelSize, 0.0)).r * heightScale * 2.0 - 1.0;
	float right = texture(heightMap, HeightMapCoords + vec2( uTexelSize, 0.0)).r * heightScale * 2.0 - 1.0;
	float up    = texture(heightMap, HeightMapCoords + vec2(0.0,  uTexelSize)).r * heightScale * 2.0 - 1.0;
	float down  = texture(heightMap, HeightMapCoords + vec2(0.0, -uTexelSize)).r * heightScale * 2.0 - 1.0;
	vec3 normal = normalize(vec3(left - right, 2.0, up-down));
	float slope = max(dot(normal, vec3(0.0f,1.0f,0.0f)), 0.0f); // slope from up
	
	vec4 col = vec4(0.7176f, 0.5922f, 0.4353f, 1.0f); //vec4(h, h, h, 1.0);
	
	if(Height < 0.0){
		col = vec4(0.098f,0.137f,0.659f, 1.0f);
	}else if(slope < radians(24.0)){
		col = vec4(0.659,0.631,0.569, 1.0f);//rock
	}else if(Height > (-59.0 * abs(latitude) + 4700)){// snow line
		col = vec4(0.8902f, 0.9059f, 0.9294f, 1.0f);//snow
	}else if(Height < 8.0f){ 
		col = vec4(0.965,0.843,0.69, 1.0f); // sand
	}else if(Height < 2750){
		//1158m/ 3997m = 0.29
		//(x-0.1829)/(0.29-0.1829)  blend from grass to tree from 731m to 1158m
		col = mix(vec4(0.384,0.451,0.255f, 1.0f), vec4(0.1647f, 0.1961f ,0.1569f, 1.0f),clamp((h-0.1829)/(0.29-0.1829), 0.0f, 1.0f));
		//col = vec4( 0.1647f, 0.1961f ,0.1569f, 1.0f); // trees
	}else{
		col = vec4(0.659,0.631,0.569, 1.0f);//rock
	}
	float diff = max(0.4+0.6*dot(normal, sunDirection.xyz), 0.0);
	float depth = LinearizeDepth(gl_FragCoord.z) / farPlane;
	//float gamma = 2.2; //TODO add gamma correction
	
	float fogFactor = 1/pow(2,pow(depth*5.0f,1.4));
	vec3 fogColor = vec3(0.788f,0.906f,1.0f);
    //FragColor = vec4(vec3(depth), 1.0);
	FragColor = vec4(mix(fogColor,((col.rgb * 0.05) + diff) * col.rgb, fogFactor),1.0);
	//FragColor = vec4(normal, 1.0f);
}
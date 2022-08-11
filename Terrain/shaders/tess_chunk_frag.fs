// fragment shader
#version 410 core

uniform sampler2D heightMap;  // the texture corresponding to our height map
uniform float heightScale; // (number of units (or maximum tiles) / texture size in meters) * maximum height of height map from 0
uniform float uTexelSize; // 1/total size of terrain

in float Height; //Height from Evaluation Shader
in vec2 HeightMapCoords;
in vec4 mpvResult;

out vec4 FragColor;

vec3 lightDir = normalize(vec3(0.0f, 0.5f, -0.5f));

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
	
	FragColor = vec4(mpvResult.xy/mpvResult.w, 1.0f, 1.0f);//vec4((vec3(0.207f, 0.318f, 0.361f) + diff) * col.rgb,1.0f);
}
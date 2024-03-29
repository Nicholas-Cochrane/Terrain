#version 420 core

layout (std140) uniform lowUpdateShared
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

out vec4 FragColor;
in vec4 FragPos;
in vec4 Normal;
#define PI 3.1415926535897932384626433832795

float LinearizeDepth(float depth) 
{
    // new 0 to 1 linerized depth value	
	return nearPlane * farPlane / (farPlane + depth * (nearPlane - farPlane));
}

void main()
{
	
	vec3 viewPosition = vec3(0.0, playerPos.y, 0.0);
	vec4 worldPostion = playerPos + FragPos;
	vec3 color = vec3(0.0,0.412,0.58);
	vec3 ambient = 0.05 * color;
	
	float waveLength = 100;
	float amplitude = 1;
	float speed = 50;

	float k = 2 * PI / waveLength;
	float f = k * (worldPostion.x - speed * time);

	vec3 tangent = normalize(vec3(1, k * amplitude * cos(f), 0));
	vec3 normal = vec3(-tangent.y, tangent.x, 0) * Normal.xyz;
	
	float diff = max(0.4+0.6*dot(sunDirection.xyz, normal),0.0);
	vec3 diffuse = diff * color;
	
	vec3 viewDir = normalize(viewPosition - FragPos.xyz);
	vec3 halfwayDir = normalize(sunDirection.xyz + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
	vec3 specular = vec3(0.3) * spec; // assuming bright white light color
	
	//float gamma = 2.2;//TODO add gamma correction?
	
	float depth = LinearizeDepth(gl_FragCoord.z) / farPlane; 
	float fogFactor = 1/pow(2,pow(depth*5.0f,1.4));
	vec3 fogColor = vec3(0.788f,0.906f,1.0f);
	FragColor = vec4(mix(fogColor,ambient + diffuse + specular, fogFactor),1.0);
}

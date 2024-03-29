#version 420 core
out vec4 FragColor;

layout (std140, binding = 1) uniform freqUpdateShared
{
	float time;         //0
	float windAngle;    //4
	vec4 sunDirection;  //16
	vec4 playerPos;     //32
};

in vec3 fpos;
in vec4 offsetPos;
in vec3 vertNormal;
in vec3 gNormal;
in float grassHeight;
//in vec3 debugColor;
//in float corner;

void main()
{
	
	vec3 grassColor = vec3(0.384,0.451,0.255f);
	//grassColor = vec3(grassHeight,0,0);
	vec3 skyColor = vec3(0.3f, 0.4f, 0.5f);
	vec3 AOColor = vec3(0.2f, 0.2f, 0.2f);
	vec3 normal;
	
	vec3 ambient = (0.05 * grassColor + 0.005 * skyColor)*clamp(fpos.y*fpos.y, 0, 1); // ambient * less ambient light lower down
	if(gl_FrontFacing){
		normal = vec3(vertNormal);
	}else{
		normal = vec3(-vertNormal);
	}
	
	float diff = max(0.4+0.6*dot(sunDirection.xyz, gNormal),0.0);
	vec3 groundDiffuse = diff * grassColor;
	
	diff = max(0.7+0.3*dot(sunDirection.xyz, normal),0.0);
	vec3 diffuse = diff * groundDiffuse * smoothstep(-(1.9-grassHeight),1.0,fpos.y); //TODO add ambient occlusion based on fpos and grassHeight
	
	vec3 viewDir = normalize(playerPos.xyz - offsetPos.xyz);
	vec3 halfwayDir = normalize(sunDirection.xyz + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 60.0);
	vec3 specular = vec3(0.3) * spec; // assuming bright white light color
	
	FragColor = vec4(ambient+diffuse+specular,1.0);
	/*if(corner == 1.0){
		FragColor = vec4(1,0,1,1);
	}*/
	//FragColor = vec4(debugColor,1.0);
}

#version 330 core
out vec4 FragColor;

uniform mat4 persMatrix;
uniform mat4 invPersMatrix;
uniform vec4 viewport;
uniform vec2 depthrange;
uniform vec3 sunDirection;
uniform float horizonAngle;

vec4 CalcEyeFromWindow(in vec3 windowSpace)
{
	vec3 ndcPos;
	ndcPos.xy = ((2.0 * windowSpace.xy) - (2.0 * viewport.xy)) / (viewport.zw) - 1;
	ndcPos.z = (2.0 * windowSpace.z - depthrange.x - depthrange.y) /
    (depthrange.y - depthrange.x);

	vec4 clipPos;
	clipPos.w = persMatrix[3][2] / (ndcPos.z - (persMatrix[2][2] / persMatrix[2][3]));
	clipPos.xyz = ndcPos * clipPos.w;

	return invPersMatrix * clipPos;
}

void main()
{	
	vec4 rayDir = CalcEyeFromWindow(gl_FragCoord.xyz);
	
	vec4 color = vec4(1,0,1,1);
	color = vec4(0.3f, 0.4f, 0.5f, 1.0f);
	//TODO optomize
	if(dot(normalize(rayDir.xyz), sunDirection) > 0.999){
		color = vec4(1,1,1,1);
	
	}else if(acos(dot(normalize(rayDir.xyz),vec3(0,-1,0))) < min(3.14/2-horizonAngle, 3.14/2)){
		//color = vec4(0.098f,0.137f,0.659f,1.0f);
		
		//float squaredDistToCenterOfEarth = pow(2000.0 + 6371000.0, 2);
		//float horizonDist = sqrt(squaredDistToCenterOfEarth - (6371000.0 * 6371000.0));
		vec3 normal = vec3(0,1,0);
		float diff = max(0.4+0.6*dot(sunDirection, normal),0.0);
		vec3 diffuse = diff * vec3(0.0,0.412,0.58);
		vec3 ambient = 0.05 * vec3(0.0,0.412,0.58);
		vec3 halfwayDir = normalize(sunDirection + -normalize(rayDir.xyz));  
		float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
		vec3 specular = vec3(0.3) * spec; // assuming bright white light color
		color = vec4(ambient + diffuse + specular, 1.0);
		
		float dist = normalize(rayDir.xyz).y+1.0; // very temporary bulshit
		float fogFactor = 1/pow(2,pow(dist*5.0f,1.4));
		vec4 fogColor = vec4(0.788f,0.906f,1.0f,1.0f);
		color = mix(fogColor, color, fogFactor);
	}
	
	FragColor = color;
	//FragColor = vec4(-normalize(rayDir.xyz).y,0,0,1);
	
}

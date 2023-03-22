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
	
	if(dot(normalize(rayDir.xyz), sunDirection) > 0.999){
		color = vec4(1,1,1,1);
	}else if(acos(dot(normalize(rayDir.xyz),vec3(0,-1,0))) < min(3.14/2-horizonAngle, 3.14/2)){
		color = vec4(0.098f,0.137f,0.659f,1.0f);
	}
	
	FragColor = color;
	
}

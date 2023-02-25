#version 330 core
out vec4 FragColor;
in vec4 vertexColor;

uniform float FOV;
uniform mat4 persMatrix;
uniform mat4 invPersMatrix;
uniform vec4 viewport;
uniform vec2 depthrange;

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
	vec4 temp = CalcEyeFromWindow(gl_FragCoord.xyz);
	FragColor = vec4(temp.xyz,1.0);
	
}

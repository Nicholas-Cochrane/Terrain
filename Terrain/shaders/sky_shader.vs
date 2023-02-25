#version 330 core
layout (location = 0) in vec3 clipPos;
out vec4 vertexColor;
void main()
{
	vec2 texCoord = clipPos.xy+1.0;
    gl_Position = vec4(clipPos, 1.0);
    vertexColor = vec4(texCoord,0.0,1.0);
}

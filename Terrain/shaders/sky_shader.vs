#version 420 core
layout (location = 0) in vec3 clipPos;
void main()
{
	vec2 texCoord = clipPos.xy+1.0;
    gl_Position = vec4(clipPos, 1.0);
}

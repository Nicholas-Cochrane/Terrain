// vertex shader
#version 410 core

layout (location = 0) in vec3 aPos;// vertex position
layout (location = 1) in vec2 aTex;// texture coordinate

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);//xyz -> xyzw
	
    // pass texture coordinate though
    TexCoord = aTex;
}
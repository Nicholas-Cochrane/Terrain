#version 420 core

uniform mat4 pvMatrix;

in vec3 vertexPos;

void main()
{

    gl_Position = vec4(vertexPos, 1.0);
}

#version 330 core
layout (location = 0) in vec3 apos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNorm;
out vec2 texCoord;
out vec3 normVec;

uniform mat4 pvMatrix;
uniform vec3 offsetCoords;
void main()
{
	texCoord = aTex;
	normVec = aNorm;
    gl_Position = pvMatrix * vec4(apos+ offsetCoords, 1.0);
}

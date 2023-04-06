#version 420 core
out vec4 FragColor;

in vec3 fpos;

void main()
{
   FragColor = vec4(fpos,1);
}

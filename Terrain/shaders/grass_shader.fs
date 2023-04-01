#version 330 core
out vec4 FragColor;

in vec3 fpos;

void main()
{
   FragColor = vec4(1,0,1,1);//vec4(fpos, 1.0);
}

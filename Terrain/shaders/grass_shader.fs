#version 420 core
out vec4 FragColor;

in vec3 fpos;
in vec3 vertNormal;

void main()
{
   FragColor = vec4(0,vertNormal.g,-vertNormal.g,1);
   //FragColor = vec4(-fpos.x*10,0,0,1);
}

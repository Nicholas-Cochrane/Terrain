#version 330 core
out vec4 FragColor;
in vec2 texCoord;
void main()
{
   FragColor = vec4(fract(abs(texCoord)),0.0f,1.0f);
   //FragColor = vec4(1,0,1,1);
}

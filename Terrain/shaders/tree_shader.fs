#version 330 core
out vec4 FragColor;
in vec2 texCoord;
in vec3 normVec;

uniform sampler2D barkTexture;
void main()
{
   //FragColor = vec4(fract(abs(texCoord)),0.0f,1.0f);
   //FragColor = vec4((normVec/2.0f)+1.0f,1.0f);
   FragColor = texture(barkTexture, texCoord);
}

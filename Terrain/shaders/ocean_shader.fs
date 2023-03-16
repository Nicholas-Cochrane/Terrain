#version 410 core
out vec4 FragColor;
in vec4 pos;
void main()
{
	//float intX = floor(pos.x);
	//float intZ = floor(pos.z);
	float intX = mod(floor((pos.x+1.0)/32.0), 2.0);
	float intZ = mod(floor((pos.z+1.0)/32.0), 2.0);

	FragColor = vec4(intX,0,intZ,1);

}

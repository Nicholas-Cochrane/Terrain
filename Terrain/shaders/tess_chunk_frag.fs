// fragment shader
#version 410 core

in float Height; //Height from Evaluation shader

out vec4 FragColor;

void main()
{
	float h = (Height + 16.0)/93.0f;
	vec4 col = vec4(h, h, h, 1.0);
	/*
	if(h < 0.1829f){ //731m / 3997m
		col = vec4(0.7176f, 0.5922f, 0.4353f, 1.0); // grass
	}else if(h < 0.6880f){//2750m / 3997m
		col = vec4( 0.1647f, 0.1961f ,0.1569f, 1.0f); // trees
	}else if(h < 0.8694f){//3475m / 3997m
		col = vec4(0.6627f, 0.6314f, 0.5725, 1.0f);//rock
	}else{
		col = vec4(0.8902f, 0.9059f, 0.9294f, 1.0f);//snow
	}*/
	
	FragColor = col;
}
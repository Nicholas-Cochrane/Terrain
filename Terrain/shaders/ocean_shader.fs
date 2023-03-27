#version 410 core
uniform vec3 playerPos;
uniform vec3 sunDirection;
out vec4 FragColor;
in vec4 FragPos;
in vec4 Normal;
void main()
{
	vec3 color = vec3(0.0,0.412,0.58);
	vec3 ambient = 0.05 * color;
	
	float diff = max(0.4+0.6*dot(sunDirection, Normal.xyz),0.0);
	vec3 diffuse = diff * color;
	
	vec3 viewDir = normalize(playerPos - FragPos.xyz);
	vec3 halfwayDir = normalize(sunDirection + viewDir);  
    float spec = pow(max(dot(Normal.xyz, halfwayDir), 0.0), 32.0);
	
	vec3 specular = vec3(0.3) * spec; // assuming bright white light color
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}

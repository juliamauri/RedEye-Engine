#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat3 modelNormal;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;  
out vec3 Normal;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = modelNormal * aNormal;  
	
	gl_Position = projection * view * vec4(FragPos, 1.0);
}
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;

uniform bool rotation = false;

uniform mat4 transform;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // note that we read the multiplication from right to left
	if(!rotation)
		gl_Position = projection * view * model * vec4(aPos, 1.0);
	else
		gl_Position = transform * vec4(aPos, 1.0);
		
    TexCoord = aTexCoord;
}
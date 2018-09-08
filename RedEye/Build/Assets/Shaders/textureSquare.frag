#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform bool vertexcolor = false;

uniform sampler2D ourTexture;

void main()
{
	if(!vertexcolor)
		FragColor = texture(ourTexture, TexCoord);
	else
		FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);  
}
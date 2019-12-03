#ifndef __DEFAULTSHADERS_H__
#define __DEFAULTSHADERS_H__

#define DEFVERTEXSHADER											\
"#version 330 core\n"											\
"layout(location = 0) in vec3 aPos;"							\
"layout(location = 1) in vec3 aNormal;"							\
"layout(location = 2) in vec3 aTangent;"						\
"layout(location = 3) in vec3 aBitangent;"						\
"layout(location = 4) in vec2 aTexCoord;"						\
""																\
"out vec2 TexCoord;"											\
""																\
"uniform mat4 model;"											\
"uniform mat4 view;"											\
"uniform mat4 projection;"										\
""																\
"void main()"													\
"{"																\
"	gl_Position = projection * view * model * vec4(aPos, 1.0);"	\
"	TexCoord = aTexCoord;"										\
"}\0"																  

#define DEFFRAGMENTSHADER														   \
"#version 330 core\n"															   \
"out vec4 FragColor;"															   \
""																				   \
"in vec2 TexCoord;"																   \
""																				   \
"uniform float useTexture;"														   \
"uniform sampler2D texture_diffuse1;"											   \
""																				   \
"uniform float useColor;"														   \
"uniform vec3 objectColor;"														   \
""																				   \
"void main()"																	   \
"{"																				   \
"	if (useTexture > 0.0f && useColor > 0.0f)"									   \
"		FragColor = texture(texture_diffuse1, TexCoord) * vec4(objectColor, 1.0);" \
"	else if (useTexture > 0.0f)"												   \
"		FragColor = texture(texture_diffuse1, TexCoord);"						   \
"	else if (useColor > 0.0f)"													   \
"		FragColor = vec4(objectColor, 1.0);"									   \
"}\0"

#define SKYBOXVERTEXSHADER							  \
"#version 330 core\n"								  \
"layout(location = 0) in vec3 aPos;"				  \
""													  \
"out vec3 TexCoords;"								  \
""													  \
"uniform mat4 projection;"							  \
"uniform mat4 view;"								  \
""													  \
"void main()"										  \
"{"													  \
"	TexCoords = aPos;"								  \
"	vec4 pos = projection * view * vec4(aPos, 1.0);"  \
"	gl_Position = pos.xyww;"						  \
"}\0"

#define SKYBOXFRAGMENTSHADER				  \
"#version 330 core\n"						  \
"out vec4 FragColor;"						  \
""											  \
"in vec3 TexCoords;"						  \
""											  \
"uniform samplerCube skybox;"				  \
""											  \
"void main()"								  \
"{"											  \
"	FragColor = texture(skybox, TexCoords);"  \
"}\0"



#endif // !__DEFAULTSHADERS_H__
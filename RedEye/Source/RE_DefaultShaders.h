#ifndef __DEFAULTSHADERS_H__
#define __DEFAULTSHADERS_H__

#define DEFVERTEXSHADER												\
"#version 330 core\n"												\
"layout(location = 0) in vec3 aPos;\n"								\
"layout(location = 1) in vec3 aNormal;\n"							\
"layout(location = 2) in vec3 aTangent;\n"							\
"layout(location = 3) in vec3 aBitangent;\n"						\
"layout(location = 4) in vec2 aTexCoord;\n"							\
"\n"																\
"out vec2 TexCoord;\n"												\
"\n"																\
"uniform mat4 model;\n"												\
"uniform mat4 view;\n"												\
"uniform mat4 projection;\n"										\
"\n"																\
"void main()\n"														\
"{\n"																\
"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"	\
"	TexCoord = aTexCoord;\n"										\
"}\0"																  

#define DEFFRAGMENTSHADER														       \
"#version 330 core\n"															       \
"out vec4 FragColor;\n"															       \
"\n"																				   \
"in vec2 TexCoord;\n"																   \
"\n"																				   \
"uniform float useTexture;\n"														   \
"uniform sampler2D texture_diffuse1;\n"											       \
"\n"																				   \
"uniform float useColor;\n"														       \
"uniform vec3 objectColor;\n"													       \
"\n"																			       \
"void main()\n"																	       \
"{\n"																				   \
"	if (useTexture > 0.0f && useColor > 0.0f)\n"									   \
"		FragColor = texture(texture_diffuse1, TexCoord) * vec4(objectColor, 1.0);\n"   \
"	else if (useTexture > 0.0f)\n"												       \
"		FragColor = texture(texture_diffuse1, TexCoord);\n"						       \
"	else if (useColor > 0.0f)\n"													   \
"		FragColor = vec4(objectColor, 1.0);\n"									       \
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